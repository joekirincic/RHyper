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

#ifndef TABLEAU_HYPER_IMPL_SCHEMANAME_IMPL_HPP
#define TABLEAU_HYPER_IMPL_SCHEMANAME_IMPL_HPP

namespace hyperapi {
inline std::string SchemaName::toString() const {
   if (databaseName) {
      return databaseName->toString() + "." + std::string(name.toString());
   } else {
      return name.toString();
   }
}

inline bool operator<(const SchemaName& a, const SchemaName& b) noexcept {
   if (a.getDatabaseName()) {
      if (b.getDatabaseName()) {
         return (*a.getDatabaseName() < *b.getDatabaseName()) || ((*a.getDatabaseName() == *b.getDatabaseName()) && a.getName() < b.getName());
      } else {
         return false;
      }
   } else {
      if (b.getDatabaseName()) {
         return true;
      } else {
         return a.getName() < b.getName();
      }
   }
}
}

namespace std {
/** Specialization of `std::hash` for `hyperapi::SchemaName`. */
template <>
struct hash<hyperapi::SchemaName> {
   /** Calculates the hash value of the given name. */
   size_t operator()(const hyperapi::SchemaName& name) const noexcept {
      size_t h = hash<string>()(name.getName().toString());
      if (name.getDatabaseName()) {
         h ^= hash<string>()(name.getDatabaseName()->getName().toString());
      }
      return h;
   }
};
}
#endif
