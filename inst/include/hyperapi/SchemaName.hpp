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

#ifndef TABLEAU_HYPER_SCHEMANAME_HPP
#define TABLEAU_HYPER_SCHEMANAME_HPP

#include <hyperapi/DatabaseName.hpp>
#include <hyperapi/Name.hpp>

namespace hyperapi {
/** Represents an escaped SQL schema name. */
class SchemaName {
   public:
   /** Constructor for a non-qualified schema name
     * \pre !name.isEmpty()
     */
   SchemaName(Name name) : name(std::move(name)) {}

   /** \copydoc SchemaName(Name name) */
   SchemaName(std::string name) : SchemaName(Name(name)) {}

   /**
     * \copydoc SchemaName(Name name)
     * \pre name != nullptr
     */
   SchemaName(const char* name) : SchemaName(Name(name)) {}

   /** Constructor for a qualified schema name
     * \pre !name.isEmpty()
     */
   SchemaName(DatabaseName databaseName, Name name) : databaseName(std::move(databaseName)), name(std::move(name)) {}

   /** \returns The properly quoted, escaped, and dot-separated string representation of the entire name */
   std::string toString() const;

   /** \returns The simple schema name without the optional database name prefix */
   const Name& getName() const noexcept { return name; }

   /** \returns The optional database name prefix */
   const optional<DatabaseName>& getDatabaseName() const noexcept { return databaseName; }

   /** \returns Whether this name is fully qualified, i.e., whether it has a database name */
   bool isFullyQualified() const noexcept { return databaseName.has_value(); }

   private:
   /// The nameof the database (if any)
   optional<DatabaseName> databaseName;
   /// The name of the schema
   Name name;
};

/** Stream output operator */
inline std::ostream& operator<<(std::ostream& os, const SchemaName& name) { return os << name.toString(); }

/** Smaller operator. */
inline bool operator<(const SchemaName& a, const SchemaName& b) noexcept;
/** Equality operator. */
inline bool operator==(const SchemaName& a, const SchemaName& b) noexcept { return (a.getDatabaseName() == b.getDatabaseName()) && (a.getName() == b.getName()); }
/** Greater operator. */
inline bool operator>(const SchemaName& a, const SchemaName& b) noexcept { return b < a; }
/** Not equal operator. */
inline bool operator!=(const SchemaName& a, const SchemaName& b) noexcept { return !(a == b); }
/** Smaller or equal operator. */
inline bool operator<=(const SchemaName& a, const SchemaName& b) noexcept { return !(a > b); }
/** Greater or equal operator. */
inline bool operator>=(const SchemaName& a, const SchemaName& b) noexcept { return !(a < b); }
}

#include <hyperapi/impl/SchemaName.impl.hpp>
#endif
