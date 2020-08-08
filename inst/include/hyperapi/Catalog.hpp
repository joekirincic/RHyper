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

#ifndef TABLEAU_HYPER_CATALOG_HPP
#define TABLEAU_HYPER_CATALOG_HPP

#include <hyperapi/HyperException.hpp>
#include <hyperapi/TableDefinition.hpp>
#include <hyperapi/impl/infra.hpp>
#include <hyperapi/hyperapi.h>

#include <string>
#include <unordered_set>

namespace hyperapi {

class Connection;

/**
 * The catalog class gives access to the metadata of the attached databases
 * of a connection.
 *
 * It can be used to obtain information about certain schema objects, such
 * as tables, to query the set of existing schema objects, and to manipulate
 * those objects.
 *
 * All manipulations and queries of this class could also be written
 * in raw SQL. The catalog class is merely a convenience wrapper.
 */
class Catalog final {
   private:
   /**
     * Create a new catalog on a connection.
     * \param connection  The connection for which to create the catalog.
     */
   explicit Catalog(Connection& connection) noexcept;

   public:
   Catalog(const Catalog&) = delete;
   Catalog& operator=(const Catalog&) = delete;
   Catalog(Catalog&&) = delete;
   Catalog operator=(Catalog&&) = delete;

   /**
     * Checks whether a table with the given name exists.
     * \param table  The name of the table.
     * \return Whether the table exists.
     * \throws HyperException
     * \pre getConnection().isOpen()
     * \pre If a database name is specified, it has to refer to an attached database.
     */
   bool hasTable(const TableName& table) const;

   /**
     * Gets the table definition for an existing table.
     * Note that the name of the table definition will always be fully qualified, even if the input name was not.
     * E.g., if the user queries table "foo", which resolves to the table "db1"."bar"."foo", then the resulting
     * `TableDefinition` will have the latter set as name.
     * \param tableName  The name of the table.
     * \return The table definition of the table.
     * \throws HyperException
     * \pre getConnection().isOpen()
     */
   TableDefinition getTableDefinition(const TableName& tableName) const;

   /**
     * Creates a SQL schema with the given name.
     * The method will throw if a schema with this name already exists.
     * \param schemaName  The name of the schema.
     * \pre getConnection().isOpen()
     * \throws HyperException
     */
   void createSchema(const SchemaName& schemaName) const;

   /**
     * Creates a SQL schema with the given name.
     * The method will just silently do nothing if a schema with this name already exists.
     * \param schemaName  The name of the schema.
     * \pre getConnection().isOpen()
     * \throws HyperException
     */
   void createSchemaIfNotExists(const SchemaName& schemaName) const;

   /**
     * Creates a SQL table with the given table definition.
     * The method will throw if a table with this name already exists.
     * \param table_definition  The definition of the table.
     * \pre getConnection().isOpen()
     * \throws HyperException
     */
   void createTable(const hyperapi::TableDefinition& table_definition) const;

   /**
     * Creates a SQL table with the given table definition.
     * The method will just silently do nothing if a table with this name already exists.
     * \param table_definition  The definition of the table.
     * \pre getConnection().isOpen()
     * \throws HyperException
     */
   void createTableIfNotExists(const hyperapi::TableDefinition& table_definition) const;

   /**
     * Gets the name of all schemas in the first database in the database search path.
     * Note that by default, this is the single database that is attached if only one database is attached.
     * When multiple databases are attached, the database search path is empty by default and this method will fail.
     * The user can set the database search path in this case to specify from which database to retrieve schema names.
     * \return  The schema names.
     * \throw HyperException
     * \pre getConnection().isOpen()
     */
   std::unordered_set<SchemaName> getSchemaNames() const;

   /**
     * Gets the names of all schemas in `database`.
     * \param database  The name of the database.
     * \return  The schema names.
     * \throw HyperException
     * \pre getConnection().isOpen()
     */
   std::unordered_set<SchemaName> getSchemaNames(const DatabaseName& database) const;

   /**
     * Gets the names of all tables in the given schema. If the schema name does not specify a database, the
     * first database in the schema_search_path is used.
     *
     * \param schema  The (qualified) schema name.
     * \return The table names.
     * \throw HyperException
     * \pre getConnection().isOpen()
     */
   std::unordered_set<TableName> getTableNames(const SchemaName& schema) const;

   /**
     * Creates a database file with the given path. Does not attach to it automatically.
     * If a relative path is given, it will be relative to the working directory
     * of the Hyper instance.
     * \param databasePath  The path for the new database. Has to end with ".hyper".
     * \throw HyperException
     * \pre getConnection().isOpen()
     */
   void createDatabase(const std::string& databasePath);

   /**
     * Creates a database file with the given path if it doesn't exist yet. Does not attach to it automatically.
     * If a relative path is given, it will be relative to the working directory
     * of the Hyper instance.
     * Note: This method will throw if a file with the given path exists that is not a valid hyper database.
     * \param databasePath  The path for the new database. Has to end with ".hyper".
     * \throw HyperException
     * \pre getConnection().isOpen()
     */
   void createDatabaseIfNotExists(const std::string& databasePath);

   /**
     * Detaches a database from the underlying connection.
     * \param databaseName  The name of the database.
     * \pre getConnection().isOpen()
     * \throw HyperException
     */
   void detachDatabase(const DatabaseName& databaseName);

   /**
     * Attaches a database file `databasePath` to the underlying connection under the name `databaseName`.
     * \param databasePath  The path to the database file
     * \param databaseName  The name for the database
     * \pre getConnection().isOpen()
     * \throw HyperException
     */
   void attachDatabase(const std::string& databasePath, const DatabaseName& databaseName);

   /**
     * Attaches a database file `databasePath` to the underlying connection.
     * Use the stem of the `databasePath` as name. E.g., "foo/bar.hyper" will use the name "bar"
     * \param databasePath  The path to the database file
     * \pre getConnection().isOpen()
     * \throw HyperException
     */
   void attachDatabase(const std::string& databasePath);

   /**
     * Detaches all databases from the underlying connection.
     * If this connection was the only connection that was currently attached to
     * these databases, then the database files will be unloaded, all updates will be written to disk, and
     * the database file can  be accessed externally (e.g., copied, moved, etc.).
     *
     * \pre getConnection().isOpen()
     * \throw HyperException
     */
   void detachAllDatabases();

   /**
     * Drops the database with the given path.
     * If a relative path is given, it will be relative to the working directory of the Hyper instance.
     * Dropping a database will delete the corresponding database file.
     *
     * Throws if a database file with the given path doesn't exist.
     *
     * \param databasePath  The path to the database.
     * \throw HyperException
     * \pre getConnection().isOpen()
     */
   void dropDatabase(const std::string& databasePath);

   /**
     * Drops the database with the given path.
     * If a relative path is given, it will be relative to the working directory of the Hyper instance.
     * Dropping a database will delete the corresponding database file.
     *
     * Does nothing if a database file with the given path doesn't exist.
     *
     * \param databasePath  The path to the database.
     * \throw HyperException
     * \pre getConnection().isOpen()
     */
   void dropDatabaseIfExists(const std::string& databasePath);

   /**
     * Returns the underlying connection.
     */
   Connection& getConnection() const noexcept { return m_connection; }

   private:
   /// The underlying connection
   Connection& m_connection;

   friend class Connection;
};
}

#endif
