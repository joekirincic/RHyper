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

#ifndef TABLEAU_HYPER_TABLENAME_HPP
#define TABLEAU_HYPER_TABLENAME_HPP

#include <hyperapi/DatabaseName.hpp>
#include <hyperapi/Name.hpp>
#include <hyperapi/SchemaName.hpp>

namespace hyperapi {

/** Represents an escaped SQL table name. */
class TableName {
   public:
   /** Constructor for a non-qualified table name */
   TableName(Name name) : name(std::move(name)) {}

   /** \copydoc TableName(Name name) */
   TableName(std::string name) : TableName(Name(name)) {}

   /**
     * \copydoc TableName(Name name)
     * \pre name != nullptr
     */
   TableName(const char* name) : TableName(Name(name)) {}

   /** Constructor for a partly qualified table name */
   TableName(SchemaName schemaName, Name name) : schemaName(std::move(schemaName)), name(std::move(name)) {}

   /** Constructor for a fully qualified table name */
   TableName(DatabaseName databaseName, Name schemaName, Name name) : schemaName({std::move(databaseName), std::move(schemaName)}), name(std::move(name)) {}

   /** \returns The properly quoted, escaped, and dot-separated string representation of the entire name */
   std::string toString() const;

   /** \returns The simple schema name without the optional database name prefix */
   const Name& getName() const noexcept { return name; }

   /** \returns the optional schema name prefix */
   const optional<SchemaName>& getSchemaName() const noexcept { return schemaName; }

   /** \returns the optional database name prefix of the optional schema name prefix */
   const optional<DatabaseName>& getDatabaseName() const noexcept;

   /** \returns whether this name is fully qualified, i.e., whether it has a schema name and a database name */
   bool isFullyQualified() const noexcept { return schemaName.has_value() && schemaName->isFullyQualified(); }

   private:
   /// The schema name (if any)
   optional<SchemaName> schemaName;
   /// The table name
   Name name;
};

/** Stream output operator */
inline std::ostream& operator<<(std::ostream& os, const TableName& name) { return os << name.toString(); }

/** Smaller operator. */
inline bool operator<(const TableName& a, const TableName& b) noexcept;
/** Equality operator. */
inline bool operator==(const TableName& a, const TableName& b) noexcept { return (a.getSchemaName() == b.getSchemaName()) && (a.getName() == b.getName()); }
/** Greater operator. */
inline bool operator>(const TableName& a, const TableName& b) noexcept { return b < a; }
/** Not equal operator. */
inline bool operator!=(const TableName& a, const TableName& b) noexcept { return !(a == b); }
/** Smaller or equal operator. */
inline bool operator<=(const TableName& a, const TableName& b) noexcept { return !(a > b); }
/** Greater or equal operator. */
inline bool operator>=(const TableName& a, const TableName& b) noexcept { return !(a < b); }
}

#include <hyperapi/impl/TableName.impl.hpp>
#endif
