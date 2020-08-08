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

#ifndef TABLEAU_HYPER_IMPL_TABLENAME_IMPL_HPP
#define TABLEAU_HYPER_IMPL_TABLENAME_IMPL_HPP

namespace hyperapi {
inline std::string TableName::toString() const {
   if (schemaName) {
      return schemaName->toString() + "." + std::string(name.toString());
   } else {
      return name.toString();
   }
}

inline bool operator<(const TableName& a, const TableName& b) noexcept {
   if (a.getSchemaName()) {
      if (b.getSchemaName()) {
         return (*a.getSchemaName() < *b.getSchemaName()) || ((*a.getSchemaName() == *b.getSchemaName()) && a.getName() < b.getName());
      } else {
         return false;
      }
   } else {
      if (b.getSchemaName()) {
         return true;
      } else {
         return a.getName() < b.getName();
      }
   }
}

namespace internal {
/// Helper object so that we can return a reference in TableName::getDatabaseName()
static const optional<DatabaseName> noDatabaseName = {};
}

inline const optional<DatabaseName>& TableName::getDatabaseName() const noexcept {
   return schemaName ? schemaName->getDatabaseName() : internal::noDatabaseName;
}
}

namespace std {
/** Specialization of `std::hash` for `hyperapi::TableName`. */
template <>
struct hash<hyperapi::TableName> {
   /** Calculates the hash value of the given name. */
   size_t operator()(const hyperapi::TableName& name) const noexcept {
      size_t h = hash<string>()(name.getName().toString());
      if (name.getSchemaName()) {
         h ^= hash<string>()(name.getSchemaName()->getName().toString());
      }
      if (name.getDatabaseName()) {
         h ^= hash<string>()(name.getDatabaseName()->getName().toString());
      }
      return h;
   }
};
}

#endif
