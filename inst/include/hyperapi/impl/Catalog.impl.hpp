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

#ifndef TABLEAU_HYPER_IMPL_CATALOG_IMPL_HPP
#define TABLEAU_HYPER_IMPL_CATALOG_IMPL_HPP

#include <hyperapi/Catalog.hpp>
#include <hyperapi/impl/infra.hpp>
#include <string>

namespace hyperapi {

inline Catalog::Catalog(Connection& con) noexcept
   : m_connection(con) {
}

inline void Catalog::createSchema(const hyperapi::SchemaName& schemaName) const {
   optional<DatabaseName> dbName = schemaName.getDatabaseName();
   const char* dbNameCStr = dbName ? dbName->getName().getUnescaped().c_str() : nullptr;
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   if (hyper_error_t* error = hyper_create_schema(m_connection.m_handle, dbNameCStr, schemaName.getName().getUnescaped().c_str(), true))
      throw internal::makeHyperException(error);
}

inline void Catalog::createSchemaIfNotExists(const hyperapi::SchemaName& schemaName) const {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   optional<DatabaseName> dbName = schemaName.getDatabaseName();
   const char* dbNameCStr = dbName ? dbName->getName().getUnescaped().c_str() : nullptr;
   if (hyper_error_t* error = hyper_create_schema(m_connection.m_handle, dbNameCStr, schemaName.getName().getUnescaped().c_str(), false))
      throw internal::makeHyperException(error);
}

inline void Catalog::createTable(const hyperapi::TableDefinition& table_definition) const {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   if (hyper_error_t* error = hyper_create_table(m_connection.m_handle, internal::HyperTableDefinition(table_definition).get(), true))
      throw internal::makeHyperException(error);
}

inline void Catalog::createTableIfNotExists(const hyperapi::TableDefinition& table_definition) const {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   if (hyper_error_t* error = hyper_create_table(m_connection.m_handle, internal::HyperTableDefinition(table_definition).get(), false))
      throw internal::makeHyperException(error);
}

namespace internal {
/// RAII helper for hyper_table_definition_t that guarantees that the resource is cleaned up at the end of tis scope
struct hyper_table_definition_t_deleter {
   void operator()(hyper_table_definition_t* tableDefinition) { hyper_destroy_table_definition(tableDefinition); }
};
using scoped_hyper_table_definition_t = std::unique_ptr<hyper_table_definition_t, hyper_table_definition_t_deleter>;
}

inline TableDefinition Catalog::getTableDefinition(const TableName& t) const {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   const char* tableName = t.getName().getUnescaped().c_str();
   const char* schemaName = t.getSchemaName() ? t.getSchemaName()->getName().getUnescaped().c_str() : "";
   const char* databaseName = t.getDatabaseName() ? t.getDatabaseName()->getName().getUnescaped().c_str() : "";

   hyper_table_definition_t* raw_table_def;
   if (hyper_error_t* error = hyper_get_table_definition(m_connection.m_handle, databaseName, schemaName, tableName, &raw_table_def)) {
      throw internal::makeHyperException(error);
   }
   internal::scoped_hyper_table_definition_t table_def(raw_table_def);

   // Assemble the Table definition (copy information from C to C++)
   const char* databaseNameNew = hyper_table_definition_database_name(table_def.get());
   const char* schemaNameNew = hyper_table_definition_schema_name(table_def.get());
   const char* tableNameNew = hyper_table_definition_table_name(table_def.get());

   TableName name = internal::isCStrEmptyOrNull(databaseNameNew) ? TableName(schemaNameNew, tableNameNew) : TableName(databaseNameNew, schemaNameNew, tableNameNew);

   TableDefinition tableDefinition(name);
   hyper_table_persistence_t type = hyper_table_definition_table_persistence(table_def.get());
   switch (type) {
      case hyper_table_persistence_t::HYPER_PERMANENT:
         tableDefinition.setPersistence(Persistence::Permanent);
         break;
      case hyper_table_persistence_t::HYPER_TEMPORARY:
         tableDefinition.setPersistence(Persistence::Temporary);
         break;
      default:
         throw internal::makeHyperException("Unexpected persistence type in `Catalog::getTableDefinition()`", "", ContextId(0x52bab947u));
   }

   size_t columnCount = hyper_table_definition_column_count(table_def.get());
   for (hyper_field_index_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
      Nullability nullability = hyper_table_definition_column_is_nullable(table_def.get(), columnIndex) ? Nullability::Nullable : Nullability::NotNullable;
      TypeTag tag = static_cast<TypeTag>(hyper_table_definition_column_type_tag(table_def.get(), columnIndex));
      hyper_type_modifier_t modifier = hyper_table_definition_column_type_modifier(table_def.get(), columnIndex);
      uint32_t oid = hyper_table_definition_column_type_oid(table_def.get(), columnIndex);
      const char* collationPtr = hyper_table_definition_column_collation(table_def.get(), columnIndex);
      std::string collation = collationPtr ? std::string(collationPtr) : std::string();
      std::string columnName = hyper_table_definition_column_name(table_def.get(), columnIndex);

      tableDefinition.addColumn({std::move(columnName), {tag, oid, modifier}, std::move(collation), nullability});
   }

   return tableDefinition;
}

namespace internal {
// RAII guard to destroy the string list
struct string_list_guard {
   hyper_string_list_t* list = nullptr;

   ~string_list_guard() {
      if (list) {
         hyper_string_list_destroy(list);
      }
   }
};
}

inline std::unordered_set<TableName> Catalog::getTableNames(const SchemaName& schema) const {
   internal::string_list_guard string_list;
   auto& dbQName = schema.getDatabaseName();
   const char* dbName = dbQName ? dbQName->getName().getUnescaped().c_str() : nullptr;
   hyper_error_t* error = hyper_get_table_names(m_connection.m_handle, dbName, schema.getName().getUnescaped().c_str(), &string_list.list);

   if (error) {
      throw internal::makeHyperException(error);
   }

   // Copy over the result from C to C++
   size_t result_size = hyper_string_list_size(string_list.list);
   std::unordered_set<TableName> result;
   result.reserve(result_size);
   for (int i = 0; i < static_cast<int>(result_size); ++i) {
      result.emplace(schema, hyper_string_list_at(string_list.list, i));
   }
   return result;
}

namespace internal {
inline std::unordered_set<SchemaName> getSchemaNames(hyper_connection_t* connection, optional<DatabaseName> database) {
   internal::string_list_guard string_list;
   const char* dbName = database ? database->getName().getUnescaped().c_str() : "";
   hyper_error_t* error = hyper_get_schema_names(connection, dbName, &string_list.list);

   if (error) {
      throw internal::makeHyperException(error);
   }

   // Copy over the result from C to C++
   size_t result_size = hyper_string_list_size(string_list.list);
   std::unordered_set<SchemaName> result;
   result.reserve(result_size);
   for (int i = 0; i < static_cast<int>(result_size); ++i) {
      if (database) {
         result.emplace(*database, hyper_string_list_at(string_list.list, i));
      } else {
         result.emplace(hyper_string_list_at(string_list.list, i));
      }
   }
   return result;
}
}

inline std::unordered_set<SchemaName> Catalog::getSchemaNames() const {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   return internal::getSchemaNames(m_connection.m_handle, {});
}

inline std::unordered_set<SchemaName> Catalog::getSchemaNames(const DatabaseName& database) const {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   return internal::getSchemaNames(m_connection.m_handle, database);
}

inline bool Catalog::hasTable(const TableName& t) const {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   const char* tableName = t.getName().getUnescaped().c_str();
   const char* schemaName = t.getSchemaName() ? t.getSchemaName()->getName().getUnescaped().c_str() : "";
   const char* databaseName = t.getDatabaseName() ? t.getDatabaseName()->getName().getUnescaped().c_str() : "";

   bool result;
   hyper_error_t* error = hyper_has_table(m_connection.m_handle, databaseName, schemaName, tableName, &result);

   if (error) {
      throw internal::makeHyperException(error);
   }
   return result;
}

namespace internal {
/**
 * Creates a database file with the given path. Does not attach to it automatically.
 * If a relative path is given, it will be relative to the working directory
 * of the Hyper instance.
 * \param databasePath  The path for the new database. Has to end with ".hyper".
 * \param c  The connection handle.
 * \param failIfExists  Whether the method throws if the database already exists.
 * \throw HyperException
 * \pre getConnection().isOpen()
 */
inline void createDatabase(const std::string& databasePath, hyper_connection_t* c, bool failIfExists) {
   if (hyper_error_t* error = hyper_create_database(c, databasePath.c_str(), failIfExists)) {
      throw internal::makeHyperException(error);
   }
}

/**
 * Drops the database with the given path (helper method).
 * \param path  The path to the database.
 * \param c  The connection handle.
 * \param failIfNotExists  Whether the method throws if the database does not exist.
 * \throws HyperException
 */
inline void dropDatabase(const std::string& path, hyper_connection_t* c, bool failIfNotExists) {
   if (hyper_error_t* error = hyper_drop_database(c, path.c_str(), failIfNotExists)) {
      throw internal::makeHyperException(error);
   }
}
}

inline void Catalog::createDatabase(const std::string& databasePath) {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   internal::createDatabase(databasePath, m_connection.m_handle, true);
}

inline void Catalog::createDatabaseIfNotExists(const std::string& databasePath) {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   internal::createDatabase(databasePath, m_connection.m_handle, false);
}

inline void Catalog::dropDatabase(const std::string& databasePath) {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   internal::dropDatabase(databasePath, m_connection.m_handle, true);
}

inline void Catalog::dropDatabaseIfExists(const std::string& databasePath) {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   internal::dropDatabase(databasePath, m_connection.m_handle, false);
}

inline void Catalog::detachAllDatabases() {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");

   if (hyper_error_t* error = hyper_detach_all_databases(m_connection.m_handle)) {
      throw internal::makeHyperException(error);
   }
}

inline void Catalog::detachDatabase(const DatabaseName& databaseName) {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   if (hyper_error_t* error = hyper_detach_database(m_connection.m_handle, databaseName.getName().getUnescaped().c_str())) {
      throw internal::makeHyperException(error);
   }
}

inline void Catalog::attachDatabase(const std::string& databasePath) {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   if (hyper_error_t* error = hyper_attach_database(m_connection.m_handle, databasePath.c_str(), nullptr)) {
      throw internal::makeHyperException(error);
   }
}

inline void Catalog::attachDatabase(const std::string& databasePath, const DatabaseName& databaseName) {
   HYPER_PRECONDITION_MSG(m_connection.isOpen(), "Underlying connection is closed.");
   if (hyper_error_t* error = hyper_attach_database(m_connection.m_handle, databasePath.c_str(), databaseName.getName().getUnescaped().c_str())) {
      throw internal::makeHyperException(error);
   }
}
}

#endif
