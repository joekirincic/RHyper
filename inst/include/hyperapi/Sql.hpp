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

#ifndef TABLEAU_HYPER_SQL_HPP
#define TABLEAU_HYPER_SQL_HPP

#include "string_view.hpp"
#include <string>

namespace hyperapi {
/**
  * Escapes the given string for safe usage in SQL query or command strings as a string literal.
  * \param input  The string literal that should be escaped.
  * \throws bad_alloc
  * \return The escaped identifier.
  */
std::string escapeStringLiteral(string_view input);

/**
  * Escapes the given string for safe usage in SQL query or command strings as an identifier.
  * \param input  The identifier that should be escaped.
  * \throws bad_alloc
  * \return The escaped identifier.
  */
std::string escapeName(string_view input);
}
#include <hyperapi/impl/Sql.impl.hpp>
#endif
