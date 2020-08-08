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

#ifndef TABLEAU_HYPER_NAME_HPP
#define TABLEAU_HYPER_NAME_HPP

#include "optional.hpp"
#include "string_view.hpp"
#include <stdexcept>
#include <string>

namespace hyperapi {

/**
 * Represents an escaped SQL name.
 */
class Name final {
   private:
   /// The name
   std::string name;
   /// The unescaped name
   std::string unescaped;

   public:
   /**
     * Constructs a properly quoted and escaped SQL name.
     * The name will be equal to the return value of calling hyperapi::escapeName(name).
     * \param name  The name
     * \pre !name.empty()
     */
   Name(std::string name);

   /**
     * \copydoc Name(std::string name)
     * \pre name != nullptr
     */
   Name(const char* name);

   /** \returns Returns the properly quoted and escaped string representation of this name. */
   const std::string& toString() const { return name; }

   /** \returns Whether the name is empty */
   bool isEmpty() const noexcept { return name.empty(); }

   /**
     * \returns The original unescaped string that was passed to the `Name` constructor.
     *
     * Do not use the result of this method in SQL, as it is prone to SQL injection.
     * This method should be used for use-cases where the original name is more readable (e.g., logging).
     */
   const std::string& getUnescaped() const { return unescaped; }
};

/** Stream output operator */
inline std::ostream& operator<<(std::ostream& os, const Name& name) { return os << name.toString(); }

/** Smaller operator. */
inline bool operator<(const Name& a, const Name& b) noexcept { return a.getUnescaped() < b.getUnescaped(); }
/** Equality operator. */
inline bool operator==(const Name& a, const Name& b) noexcept { return a.getUnescaped() == b.getUnescaped(); }
/** Greater operator. */
inline bool operator>(const Name& a, const Name& b) noexcept { return b < a; }
/** Not equal operator. */
inline bool operator!=(const Name& a, const Name& b) noexcept { return !(a == b); }
/** Smaller or equal operator. */
inline bool operator<=(const Name& a, const Name& b) noexcept { return !(a > b); }
/** Greater or equal operator. */
inline bool operator>=(const Name& a, const Name& b) noexcept { return !(a < b); }
}
#include <hyperapi/impl/Name.impl.hpp>

#endif
