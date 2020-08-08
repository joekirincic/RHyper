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

#ifndef TABLEAU_HYPER_IMPL_HYPEREXCEPTION_IMPL_HPP
#define TABLEAU_HYPER_IMPL_HYPEREXCEPTION_IMPL_HPP

#include <hyperapi/HyperException.hpp>

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <string>

namespace hyperapi {

namespace internal {
/**
 * Converts the given c-string to a std::string and replaces all '\\n' with '\\n\\t'.
 * \param msg  The c-string.
 * \return  The altered std::string.
 */
inline std::string convertToIndentedString(const char* msg) {
   std::ostringstream ss;
   while (*msg != '\0') {
      if (*msg == '\n') {
         ss << "\n\t";
      } else {
         ss << *msg;
      }
      ++msg;
   }
   return ss.str();
}

/**
 * Creates a Hyper error.
 * \param message  The error message.
 * \param hint  The error hint.
 * \param id  The context id.
 * \return The error object.
 */
inline HyperException makeHyperException(const std::string& message, const std::string& hint, ContextId id) {
   hyper_error_t* newError = hyper_error_create(id.getValue());
   if (!newError) {
      throw std::bad_alloc();
   }

   auto setField = [&](hyper_error_field_key key, const std::string& value) {
      hyper_error_field_value val;
      val.discriminator = 1;
      val.value.string = value.c_str();
      if (hyper_error_t* error = hyper_error_set_field(newError, key, val)) {
         throw internal::makeHyperException(error);
      }
   };

   if (!message.empty()) {
      setField(HYPER_ERROR_FIELD_MESSAGE, message);
   }

   if (!hint.empty()) {
      setField(HYPER_ERROR_FIELD_HINT_MESSAGE, hint);
   }

   return HyperException(newError);
}

inline HyperException makeHyperException(hyper_error_t* error) {
   return HyperException(error);
}

/**
 * Get the sql state from a hyper exception
 * \param ex  The exception.
 * \return The sql state.
 */
inline const char* getSqlState(const hyperapi::HyperException& ex) {
   hyper_error_field_value value;
   if (const hyper_error_t* error = hyper_error_get_field(ex.m_error.get(), HYPER_ERROR_FIELD_SQL_STATE, &value))
      return {};
   return value.value.string;
}

/**
 * Get the detail part of the message from a hyper exception
 * \param ex  The exception.
 * \return The detail part of the message.
 */
inline const char* getDetail(const hyperapi::HyperException& ex) {
   hyper_error_field_value value;
   if (const hyper_error_t* error = hyper_error_get_field(ex.m_error.get(), HYPER_ERROR_FIELD_DETAIL, &value))
      return {};
   return value.value.string;
}

/**
 * Get the error code from a hyper exception.
 * \param ex  The exception.
 * \return The error code.
 */
inline int getErrorCode(const hyperapi::HyperException& ex) {
   hyper_error_field_value value;
   if (const hyper_error_t* error = hyper_error_get_field(ex.m_error.get(), HYPER_ERROR_FIELD_ERROR_CODE, &value))
      return {};
   return value.value.integer;
}

/**
 * Get the error category from a hyper exception.
 * \param ex  The exception.
 * \return The error code.
 */
inline int getErrorCategory(const hyperapi::HyperException& ex) {
   hyper_error_field_value value;
   if (const hyper_error_t* error = hyper_error_get_field(ex.m_error.get(), HYPER_ERROR_FIELD_ERROR_CATEGORY, &value))
      return {};
   return value.value.integer;
}
}

inline HyperException& HyperException::operator=(const HyperException& other) noexcept {
   // We intentionally don't copy the `stringRepresentation` since it is not necessary and this method should be noexcept
   if (this != &other) {
      m_error = other.m_error;
   }
   return *this;
}

inline HyperException::HyperException(const HyperException& other) noexcept {
   // We intentionally don't copy the `stringRepresentation` since it is not necessary and this method should be noexcept
   m_error = other.m_error;
}

inline HyperException::HyperException(hyper_error_t* error)
   : std::exception(), m_error(error, hyper_error_destroy) {
}

inline HyperException::HyperException(hyper_error_t* error, const std::shared_ptr<const hyper_error_t> top_level_error)
   : std::exception(),
     // This shared_ptr will keep `top_level_error` alive while pointing to `error` (= aliasing feature of std::shared_ptr).
     m_error(std::move(top_level_error), error) {
}

inline optional<HyperException> HyperException::getCause() const noexcept {
   if (getCauseHandle()) {
      return HyperException(getCauseHandle(), m_error);
   } else {
      return {};
   }
}

inline const char* HyperException::getStringField(hyper_error_field_key key) const noexcept {
   hyper_error_field_value value;
   if (const hyper_error_t* error = hyper_error_get_field(m_error.get(), key, &value))
      abort(); // logic error
   assert(value.discriminator == 1);
   return value.value.string;
}

inline int HyperException::getIntField(hyper_error_field_key key) const noexcept {
   hyper_error_field_value value;
   if (const hyper_error_t* error = hyper_error_get_field(m_error.get(), key, &value))
      abort(); // logic error
   assert(value.discriminator == 0);
   return value.value.integer;
}

inline uint32_t HyperException::getUIntField(hyper_error_field_key key) const noexcept {
   hyper_error_field_value value;
   if (const hyper_error_t* error = hyper_error_get_field(m_error.get(), key, &value))
      abort(); // logic error
   assert(value.discriminator == 3);
   return value.value.uinteger;
}

inline hyper_error_t* HyperException::getCauseHandle() const noexcept {
   hyper_error_field_value value;
   if (const hyper_error_t* error = hyper_error_get_field(m_error.get(), HYPER_ERROR_FIELD_CAUSE, &value))
      return nullptr;
   return reinterpret_cast<hyper_error_t*>(value.value.pointer);
}

/**
 * Prints the given `HyperException` to the given output stream, including all error causes.
 *
 * \param os  The output stream.
 * \param exception  The `HyperException` to print.
 */
inline void printHyperException(std::ostream& os, const HyperException& exception) {
   // Print the primary message, if available.
   const char* message = exception.getMainMessage();
   if ((message != nullptr) && (*message != '\0')) {
      os << internal::convertToIndentedString(message) << '\n';
   }

   // Print the hint message, if available.
   const char* hint = exception.getHint();
   if ((hint != nullptr) && (*hint != '\0')) {
      os << "Hint: " << internal::convertToIndentedString(hint) << '\n';
   }

   // Print context id
   std::ios_base::fmtflags f(os.flags());
   os << "Context: 0x" << std::hex << exception.getContextId().getValue() << std::dec;
   os.flags(f);

   // Print the cause, if available.
   if (exception.getCause()) {
      os << "\n\nCaused by:\n";
      printHyperException(os, *exception.getCause());
   }
}
//---------------------------------------------------------------------------
inline std::string HyperException::toString() const {
   std::ostringstream ss;
   printHyperException(ss, *this);
   return ss.str();
}

inline const char* HyperException::what() const noexcept {
   try {
      if (stringRepresentation.empty()) {
         stringRepresentation = toString();
      }
      return stringRepresentation.c_str();
   } catch (...) {
      return "hyperapi::HyperException";
   }
}
}

#endif
