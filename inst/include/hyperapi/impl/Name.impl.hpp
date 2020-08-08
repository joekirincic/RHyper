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

#ifndef TABLEAU_HYPER_IMPL_NAME_IMPL_HPP
#define TABLEAU_HYPER_IMPL_NAME_IMPL_HPP

#include <cassert>
#include <hyperapi/Sql.hpp>
#include <hyperapi/impl/infra.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <hyperapi/hyperapi.h>

namespace hyperapi {
inline Name::Name(std::string name)
   : name(hyperapi::escapeName(name)), unescaped(std::move(name)) {
   HYPER_PRECONDITION_MSG(!this->unescaped.empty(), "Name(std::string name) called with empty string");
}

namespace internal {
static std::string checkNameString(const char* name) {
   HYPER_PRECONDITION_MSG(name != nullptr, "Name(const char*) called with nullptr");
   return name;
}
}

inline Name::Name(const char* name) : Name(internal::checkNameString(name)) {}
}

namespace std {
/** Specialization of `std::hash` for `hyperapi::Name`. */
template <>
struct hash<hyperapi::Name> {
   /** Calculates the hash value of the given name. */
   size_t operator()(const hyperapi::Name& name) const noexcept { return hash<string>()(name.toString()); }
};
}
#endif
