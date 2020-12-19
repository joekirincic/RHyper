//---------------------------------------------------------------------------
//
// This file is the copyrighted property of Tableau Software and is protected
// by registered patents and other applicable U.S. and international laws and
// regulations.
//
// Unlicensed use of the contents of this file is prohibited. Please refer to
// the NOTICES.txt file for further details.
//
//---------------------------------------------------------------------------

#ifndef TABLEAU_HYPER_IMPL_CONNECTION_IMPL_HPP
#define TABLEAU_HYPER_IMPL_CONNECTION_IMPL_HPP

#include <hyperapi/Catalog.hpp>
#include <hyperapi/HyperException.hpp>
#include <hyperapi/impl/Parameters.hpp>

#include <sstream>
#include <string>
#include <vector>

namespace hyperapi {

inline Connection::~Connection() noexcept {
   close();
}

inline Connection::Connection(Connection&& other) noexcept
   : m_handle(internal::exchange(other.m_handle, nullptr)), m_catalog(new Catalog(*this)) {
}

inline Connection::Connection(const Endpoint& endpoint, const std::unordered_map<std::string, std::string>& parameters)
   : Connection(endpoint, std::string{}, CreateMode::None, parameters) {
}

inline Connection::Connection(
   const Endpoint& endpoint, const std::string& databasePath, CreateMode createMode, const std::unordered_map<std::string, std::string>& parameters)
   : m_catalog(new Catalog(*this)) {
   internal::Parameters p = internal::createConnectionParameters(nullptr);

   p.set("endpoint", endpoint.getConnectionDescriptor().c_str());

   if (!databasePath.empty()) {
      p.set("dbname", databasePath.c_str());
   }

   if (!endpoint.getUserAgent().empty()) {
      p.set("user_agent", endpoint.getUserAgent().c_str());
   }
   p.set("api_language", "C++");

   for (const auto& parameter : parameters) {
      p.set(parameter.first.c_str(), parameter.second.c_str());
   }

   if (hyper_error_t* error = hyper_connect(p.m_handle, &m_handle, static_cast<hyper_create_mode_t>(createMode)))
      throw internal::makeHyperException(error);
}

inline Connection& Connection::operator=(Connection&& other) noexcept {
   if (this != &other) {
      close();

      m_handle = internal::exchange(other.m_handle, nullptr);
      m_catalog = std::unique_ptr<Catalog>(new Catalog(*this));
   }

   return *this;
}

inline void Connection::close() noexcept {
   if (m_handle) { // Lock so a concurrent cancel works
      std::lock_guard<std::mutex> lock(m_mutex);
      hyper_disconnect(m_handle);
      m_handle = nullptr;
   }
}

inline void Connection::cancel() noexcept {
   // Lock so no-one closes while we are cancelling.
   std::lock_guard<std::mutex> lock(m_mutex);
   if (m_handle) {
      if (hyper_error_t* error = hyper_cancel(m_handle)) {
         // TODO 923423: Log swallowed errors?
         // Swallow any errors, as the user is not able to react anyway
         HyperException he = internal::makeHyperException(error);
      }
   }
}

inline Catalog& Connection::getCatalog() noexcept {
   HYPER_PRECONDITION_MSG(isOpen(), "The connection is closed.");
   return *m_catalog;
}

inline Result Connection::executeQuery(const std::string& sql) {
   HYPER_PRECONDITION_MSG(isOpen(), "The connection is closed.");
   hyper_rowset_t* rowset = nullptr;

   if (hyper_error_t* error = hyper_execute_query(m_handle, sql.c_str(), &rowset)) {
      throw internal::makeHyperException(error);
   }

   return Result{rowset, *this};
}

inline int64_t Connection::executeCommand(const std::string& sql) {
   HYPER_PRECONDITION_MSG(isOpen(), "The connection is closed.");
   int affectedRowCount = -1;
   if (hyper_error_t* error = hyper_execute_command(m_handle, sql.c_str(), &affectedRowCount)) {
      throw internal::makeHyperException(error);
   }

   return affectedRowCount;
}

template <typename T>
T Connection::executeScalarQuery(const std::string& sql) {
   HYPER_PRECONDITION_MSG(isOpen(), "The connection is closed.");
   static_assert(!std::is_same<T, string_view>::value, "executeScalarQuery() closes the result and cannot return non-owning types. Try std::string instead.");
   static_assert(!std::is_same<T, optional<string_view>>::value, "executeScalarQuery() closes the result and cannot return non-owning types. Try std::string instead.");
   static_assert(!std::is_same<T, ByteSpan>::value, "executeScalarQuery() closes the result and cannot return non-owning types. Try std::vector<uint8_t> instead.");
   static_assert(!std::is_same<T, optional<ByteSpan>>::value, "executeScalarQuery() closes the result and cannot return non-owning types. Try std::vector<uint8_t> instead.");
   static_assert(!std::is_same<T, Value>::value, "executeScalarQuery() closes the result and cannot return non-owning types. Try an explicit type instead.");
   Result result = executeQuery(sql);
   if (result.getSchema().getColumnCount() != 1) {
      std::stringstream ss;
      ss << "Scalar queries must return exactly one column, but " << result.getSchema().getColumnCount() << " were returned.";
      throw internal::makeHyperException(ss.str(), "", ContextId(0x93b5fde0u));
   }
   auto rowIterator = begin(result);
   if (rowIterator == end(result)) {
      throw internal::makeHyperException("Scalar query didn't return a row.", "", ContextId(0xe8a0a244u));
   }
   T val = rowIterator->get<T>(0);
   if ((++rowIterator) != end(result)) {
      throw internal::makeHyperException("Scalar query returned more than one row.", "", ContextId(0xa3b5270eu));
   }
   return val;
}

inline bool Connection::isReady() {
   HYPER_PRECONDITION_MSG(isOpen(), "The connection is closed.");
   return hyper_connection_is_ready(m_handle);
}

namespace internal {

/// Expose the handle of the connection
inline hyper_connection_t* getHandle(Connection& c) noexcept {
   return c.m_handle;
}

/**
 * Pings a Hyper Process that is specified by the given parameters.
 * \param parameters  The parameters.
 * \return  The ping status.
 * \throw HyperException  If the ping failed.
 */
inline hyper_ping_status_t ping(const Parameters& parameters) {
   hyper_ping_status_t ping_status;
   if (hyper_error_t* error = hyper_ping(parameters.m_handle, &ping_status)) {
      throw internal::makeHyperException(error);
   }

   return ping_status;
}

/**
 * Pings the Hyper Process at the given endpoint.
 * \param endpoint  The endpoint descriptor of the Hyper Process.
 * \param user  The user that does the ping.
 * \return  The ping status.
 * \throw HyperException  If the ping failed.
 */
inline hyper_ping_status_t ping(const std::string& endpoint, const std::string& user) {
   Parameters p = hyperapi::internal::createConnectionParameters(nullptr);

   p.set("endpoint", endpoint.c_str());
   p.set("user", user.c_str());

   return ping(p);
}

/**
 * Prepares a query with the given SQL query text and registers it under the given statement name.
 *
 * Refer to the Hyper SQL documentation for the syntax and semantics of the SQL query text. //TODO(TFS 903949) link?
 *
 * \param connection  The connection.
 * \param query  The SQL query text.
 * \param statement_name  The name under which the prepared statement will be registered.
 * \pre connection.is_valid()
 * \throw hyperapi::error
 */
inline void prepareQuery(Connection& connection, const std::string& statement_name, const std::string& query) {
   if (hyper_error_t* error = hyper_prepare(getHandle(connection), statement_name.c_str(), query.c_str())) {
      throw internal::makeHyperException(error);
   }
}

/**
 * Execute a previously prepared statement.
 *
 * \param connection  The connection.
 * \param statement_name  The name under which the prepared statement was registered.
 * \param result_format  The result format code (0 = text, 1 = pg_binary, 2 = hyper_binary).
 * \throw hyperapi::error
 * \pre `connection.is_valid()`
 * result consumption.
 */
inline Result executePreparedQuery(Connection& connection, const std::string& statement_name, hyper_rowset_result_format_t result_format) {
   hyper_rowset_t* rowset = nullptr;

   if (hyper_error_t* error = hyper_execute_prepared(getHandle(connection), statement_name.c_str(), result_format, &rowset)) {
      throw internal::makeHyperException(error);
   }

   return Result(rowset, connection);
}

/**
 * Execute parameterized query.
 *
 * \param connection  The connection.
 * \param result_format  The result format code (0 = text, 1 = pg_binary, 2 =  hyper_binary).
 * \throw hyperapi::error
 * \pre `connection.is_valid()`
 * result consumption.
 */
inline Result executeQueryParams(Connection& connection, const std::string& query, hyper_rowset_result_format_t result_format) {
   hyper_rowset_t* rowset = nullptr;

   if (hyper_error_t* error = hyper_execute_query_params(getHandle(connection), query.c_str(), result_format, &rowset)) {
      throw internal::makeHyperException(error);
   }

   return Result(rowset, connection);
}

/// Set the notice receiver. Used for testing.
inline hyper_notice_receiver_t setNoticeReceiver(hyperapi::Connection& c, hyper_notice_receiver_t receiver, void* context) noexcept {
   return hyper_set_notice_receiver(getHandle(c), receiver, context);
}

/// Set the prefetch threshold. Used for testing.
inline void setPrefetchThreshold(hyperapi::Connection& c, size_t threshold) noexcept {
   return hyper_set_prefetch_threshold(getHandle(c), threshold);
}
}
}
#endif
