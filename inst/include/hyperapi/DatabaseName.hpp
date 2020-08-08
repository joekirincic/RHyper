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

#ifndef TABLEAU_HYPER_DATABASENAME_HPP
#define TABLEAU_HYPER_DATABASENAME_HPP

#include <hyperapi/Name.hpp>

namespace hyperapi {
/** Represents an escaped SQL database name. */
class DatabaseName final {
   public:
   /**
     * Constructs a properly quoted and escaped SQL Database name.
     * The name will be equal to the return value of calling hyperapi::escapeName(name).
     * \param name  The name
     * \pre !name.isEmpty()
     */
   DatabaseName(Name name) : name(std::move(name)) {}

   /** \copydoc DatabaseName(Name name) */
   DatabaseName(std::string name) : DatabaseName(Name(name)) {}

   /**
     * \copydoc DatabaseName(Name name)
     * \pre name != nullptr
     */
   DatabaseName(const char* name) : DatabaseName(Name(name)) {}

   /**
     * Returns the properly quoted and escaped string representation of this name.
     * \return The string representation.
     */
   const std::string& toString() const noexcept { return name.toString(); }

   /**
     * \return The contained name.
     */
   const Name& getName() const noexcept { return name; }

   private:
   /// The name of the database
   Name name;
};

/** Stream output operator */
inline std::ostream& operator<<(std::ostream& os, const DatabaseName& name) { return os << name.toString(); }

/** Smaller operator. */
inline bool operator<(const DatabaseName& a, const DatabaseName& b) noexcept { return a.getName() < b.getName(); }
/** Equality operator. */
inline bool operator==(const DatabaseName& a, const DatabaseName& b) noexcept { return a.getName() == b.getName(); }
/** Greater operator. */
inline bool operator>(const DatabaseName& a, const DatabaseName& b) noexcept { return b < a; }
/** Not equal operator. */
inline bool operator!=(const DatabaseName& a, const DatabaseName& b) noexcept { return !(a == b); }
/** Smaller or equal operator. */
inline bool operator<=(const DatabaseName& a, const DatabaseName& b) noexcept { return !(a > b); }
/** Greater or equal operator. */
inline bool operator>=(const DatabaseName& a, const DatabaseName& b) noexcept { return !(a < b); }
}

namespace std {
/** Specialization of `std::hash` for `hyperapi::DatabaseName`. */
template <>
struct hash<hyperapi::DatabaseName> {
   /** Calculates the hash value of the given name. */
   size_t operator()(const hyperapi::DatabaseName& name) const noexcept { return hash<string>()(name.toString()); }
};
}

#endif
