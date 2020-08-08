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

/**
 * \file
 */

#ifndef TABLEAU_HYPER_CONNECTION_HPP
#define TABLEAU_HYPER_CONNECTION_HPP

#include <functional>
#include <hyperapi/Catalog.hpp>
#include <hyperapi/HyperException.hpp>
#include <hyperapi/HyperProcess.hpp>
#include <hyperapi/Name.hpp>
#include <hyperapi/Result.hpp>
#include <hyperapi/impl/infra.hpp>
#include <hyperapi/optional.hpp>
#include <memory>
#include <mutex>
#include <new>
#include <string>
#include <map>
#include <hyperapi/hyperapi.h>

namespace hyperapi {

/// Database creation behavior during connection establishing.
enum class CreateMode {
   /// Do not create the database. Method will fail if database doesn't exist.
   None = HYPER_DO_NOT_CREATE,
   /// Create the database. Method will fail if the database already exists.
   Create = HYPER_CREATE,
   /// Create the database if it doesn't exist.
   CreateIfNotExists = HYPER_CREATE_IF_NOT_EXISTS,
   /// Create the database. If it already exists, drop the old one first.
   CreateAndReplace = HYPER_CREATE_AND_REPLACE
};

class Value;

/**
 * Defines a Hyper connection. Used for all interactions with Hyper.
 */
class Connection final {
   using CatalogPimpl = std::unique_ptr<Catalog>;

   public:
   /**
     * Connects to a Hyper endpoint without attaching to a database.
     * \param endpoint  The endpoint of the server to connect to.
     * \param parameters  Optional connection parameters to pass to Hyper. The available parameters are documented
     *                    <a href="https://help.tableau.com/current/api/hyper_api/en-us/reference/sql/connectionsettings.html" target="_top">
     *                    in the Tableau Hyper documentation, chapter "Connection Settings"</a>.
     *                    All parameter keys and values are expected to be passed in UTF-8 encoding.
     * \throw HyperException  If connecting failed.
     */
   Connection(const Endpoint& endpoint, const std::map<std::string, std::string>& parameters = {});

   /**
     * Connects to a Hyper endpoint and attaches to exactly one database.
     * \param endpoint  The endpoint of the server to connect to.
     * \param databasePath  The name/path of the database to connect to. The database will be attached using the stem of the `databasePath` as name.
     * \param createMode  Whether the database should be created and what to do in case of an already existing database.
     * \param parameters  Optional connection parameters to pass to Hyper. The available parameters are documented
     *                    <a href="https://help.tableau.com/current/api/hyper_api/en-us/reference/sql/connectionsettings.html" target="_top">
     *                    in the Tableau Hyper documentation, chapter "Connection Settings"</a>.
     *                    All parameter keys and values are expected to be passed in UTF-8 encoding.
     * \throw HyperException  If connecting failed or connecting to the specified endpoint is not  supported by this version of the API.
     */
   Connection(
      const Endpoint& endpoint,
      const std::string& databasePath,
      CreateMode createMode = CreateMode::None,
      const std::map<std::string, std::string>& parameters = {});

   /**
     * Constructs a `Connection` object that does not represent a connection.
     *
     * \post !isOpen()
     */
   Connection() noexcept {}

   /**
     * Destructor. Closes the connection (if open).
     */
   virtual ~Connection() noexcept;

   /** Move constructor. */
   Connection(Connection&& other) noexcept;

   /** Move assignment operator */
   Connection& operator=(Connection&& other) noexcept;

   Connection(const Connection& other) = delete;
   Connection& operator=(const Connection& other) = delete;

   /**
     * Executes a SQL query and returns the result.
     *
     * This method can be used to execute any SQL command, even if it doesn't
     * return a result. In this case, the result will be empty.
     *
     * Note that this method is flagged "no_discard", as you should never use it if
     * you don't need to look at the result. If you don't need a result, use executeCommand
     * instead.
     *
     * \param sql  The query
     * \throw  HyperException
     * \pre  isOpen()
     * \return  the query result
     * result consumption.
     */
   HYPER_API_NODISCARD Result executeQuery(const std::string& sql);

   /**
     * Executes a SQL command and returns the affected row count, if any.
     *
     * If the SQL statement is an UPDATE, INSERT, or DELETE statement, then this method
     * will return the number of affected rows, otherwise it will return -1.
     *
     * Note that this method can be used to execute any SQL command, even if it returns a result.
     * In that case, the result is discarded.
     *
     * \param sql  The query
     * \throw  HyperException
     * \pre  connection.isOpen()
     * \return  the number of affected rows, if applicable; otherwise -1
     */
   int64_t executeCommand(const std::string& sql);

   /**
     * Executes a SQL query that returns exactly one row with one column.
     *
     * The template parameter `T` has to be set to the anticipated value type of the column, which
     * is the type that the respective `getTYPE` method of the class `Row` would return.
     *
     * An exception to this rule are the types `string_view` and `ByteSpan`, because these are
     * non-owning references into the query result. When the method returns, the result is
     * already deconstructed, so we need to return an owning reference instead,
     * which is `std::string` for `string_view` and `std::vector<uint8_t>` for `ByteSpan`.
     * E.g., for the type `Text`, `Row::getText` would return `string_view`, so `T` would need
     * to be `std::string`. Using the wrong type here will cause an exception at runtime.
     *
     * Another exception to the rule is that this method generally allows larger integer types than
     * the query uses. So a query producing an `INTEGER` can be templated with an `int32_t` or
     * an `int64_t`.
     *
     * Finally, all types can be retrieved using `T = std::vector<uint8_t>`, which will
     * result in calling the `getRaw` method to get the raw binary representation of the type.
     * Note that the binary representation is not part of the supported interface, so it can change
     * between versions.
     *
     * By default, the return type of this method is `T`. If the result can be NULL however,
     * then T has to be an optional<>; otherwise,
     * a NULL value would result in an exception at runtime.
     *
     * ```
     * int                     i1 = executeScalarQuery<int>("...");
     * hyperapi::optional<int> i2 = executeScalarQuery<optional<int>(...);
     * ```
     *
     * This method will throw if the executed query doesn't exactly return one row with one column.
     *
     * \tparam T  The anticipated value type
     * \param sql  The query
     * \throw  HyperException
     * \pre  connection.isOpen()
     * \pre The requested type is compatible to the column type.
     * \return  the single result value, or if T is an optional<>, an absent value in case of a NULL query result.
     */
   template <typename T>
   T executeScalarQuery(const std::string& sql);

   /**
     * Returns the catalog of this connection.
     *
     * \pre isOpen()
     */
   Catalog& getCatalog() noexcept;

   /**
     * Issues an asynchronous cancel request for the running query on the given
     * connection.
     * This method may be called from another thread. Upon cancel, the command
     * executing the query may fail.
     * Note that this is a best-effort method that will never throw, even if the
     * connection is closed.
     * It is not guaranteed to do an actual cancel.
     */
   void cancel() noexcept;

   /**
     * Checks whether the connection is ready, i.e., if the connection
     * can be used. A connection that is not ready is currently processing
     * a query, so using it for further query methods will throw a HyperException.
     * Note that this method is not thread-safe; only use it on the same thread
     * that potentially uses the connection.
     *
     * (Note that a non-ready connection doesn't mean that the thread is in a blocking call;
     *  an open Result for example always keeps the connection busy).
     *
     * \pre connection.isOpen()
     * \return Whether the connection is ready.
     */
   bool isReady();

   /**
     * Checks whether the connection is open.
     */
   bool isOpen() const noexcept { return m_handle != nullptr; }

   /**
     * Closes the connection.
     * \post !isOpen()
     */
   virtual void close() noexcept;

   private:
   /// Mutex protecting the handle.
   std::mutex m_mutex;
   /// The underlying handle.
   hyper_connection_t* m_handle = nullptr;
   /// The catalog of the connection
   CatalogPimpl m_catalog;

   friend class Catalog;
   friend class Inserter;
   friend hyper_connection_t* internal::getHandle(Connection&) noexcept;
};
}

#include <hyperapi/impl/Catalog.impl.hpp>
#include <hyperapi/impl/Connection.impl.hpp>

#endif
