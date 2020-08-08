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

#ifndef TABLEAU_HYPER_IMPL_SQL_IMPL_HPP
#define TABLEAU_HYPER_IMPL_SQL_IMPL_HPP

#include <cassert>
#include <hyperapi/impl/infra.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <hyperapi/hyperapi.h>

namespace hyperapi {
inline std::string escapeStringLiteral(string_view input) {
   size_t result_size = hyper_quote_sql_literal(nullptr, 0, input.data(), input.size());
   if ((result_size == 0) && (input.size() != 0)) {
      throw std::bad_alloc();
   }
   std::string result;
   result.resize(result_size);
   size_t returned_size = hyper_quote_sql_literal(const_cast<char*>(result.data()), result.size(), input.data(), input.size());
   if ((returned_size == 0) && (input.size() != 0)) {
      throw std::bad_alloc();
   }
   return result;
}

inline std::string escapeName(string_view input) {
   size_t result_size = hyper_quote_sql_identifier(nullptr, 0, input.data(), input.size());
   if ((result_size == 0) && (input.size() != 0)) {
      throw std::bad_alloc();
   }
   std::string result;
   result.resize(result_size);
   size_t returned_size = hyper_quote_sql_identifier(const_cast<char*>(result.data()), result.size(), input.data(), input.size());
   if ((returned_size == 0) && (input.size() != 0)) {
      throw std::bad_alloc();
   }
   return result;
}
}

#endif
