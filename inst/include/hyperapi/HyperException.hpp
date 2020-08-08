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

#ifndef TABLEAU_HYPER_HYPEREXCEPTION_HPP
#define TABLEAU_HYPER_HYPEREXCEPTION_HPP

#include <hyperapi/impl/infra.hpp>
#include <hyperapi/optional.hpp>
#include <hyperapi/hyperapi.h>

#include <cassert>
#include <memory>
#include <string>
#include <system_error>

namespace hyperapi {

/**
 * A context id.
 *
 * Used to identify the source of an exception. Each throw statement has a unique context id that is stored in the thrown error.
 */
class ContextId final {
   const uint32_t value;

   public:
   constexpr explicit ContextId(uint32_t value) noexcept
      : value(value) {
   }
   constexpr uint32_t getValue() const noexcept { return value; }
};

namespace internal {
HyperException makeHyperException(const std::string&, const std::string&, ContextId);
}

/**
 * Defines an exception object that is thrown on failure by the functions in the Hyper API C++ library.
 */
class HyperException final : public std::exception {
   public:
   /// Copy assignment operator.
   HyperException& operator=(const HyperException& other) noexcept;
   /// Copy constructor.
   HyperException(const HyperException& other) noexcept;
   /// Move assignment operator.
   HyperException& operator=(HyperException&& other) = default;
   /// Move constructor.
   HyperException(HyperException&& other) = default;

   /**
     * Returns the message stored in the exception object, or "" if there is none. Error messages may change between HAPI versions.
     */
   const char* getMainMessage() const noexcept { return getStringField(HYPER_ERROR_FIELD_MESSAGE); }

   /**
     * Returns the hint message stored in the exception object, or "" if there is none.
     */
   const char* getHint() const noexcept { return getStringField(HYPER_ERROR_FIELD_HINT_MESSAGE); }

   /**
     * Returns the message stored in the exception object, or "" if there is none. Error messages may change between HAPI versions.
     *
     * \deprecated Use getMainMessage() instead. This function will be removed in the future.
     */
   HYPER_DEPRECATED("Use getMainMessage() instead. This function will be removed in the future.")
   const char*
   getMessage() const noexcept {
      return getMainMessage();
   }

   /**
     * Returns the hint message stored in the exception object, or "" if there is none.

     * \deprecated Use getHint() instead. This function will be removed in the future.
     */
   HYPER_DEPRECATED("Use getHint() instead. This function will be removed in the future.")
   const char*
   getHintMessage() const noexcept {
      return getHint();
   }

   /**
     * Returns whether this error has an underlying cause stored.
     *
     * \deprecated Use getCause().has_value() instead. This function will be removed in the future.
     */
   HYPER_DEPRECATED("Use getCause().has_value() instead. This function will be removed in the future.")
   bool hasCause() const noexcept { return (getCauseHandle() != nullptr); }

   /**
     * Returns the cause stored in the exception object.
     */
   optional<HyperException> getCause() const noexcept;

   /**
     * Returns the context id.
     */
   ContextId getContextId() const noexcept { return ContextId(getUIntField(HYPER_ERROR_FIELD_CONTEXT_ID)); }

   /**
     * Returns a formatted string containing the message and hint of the error and all causes.
     */
   std::string toString() const;

   /**
     * Returns a null terminated character sequence that may be used to identify the exception.
     */
   const char* what() const noexcept override;

   private:
   /**
     * Constructs a `hyperapi::HyperException` from a `hyper_error_t`.
     * Takes ownership of the given `hyper_error_t` object.
     *
     * \param error  The error handle.
     * \throw std::bad_alloc  if not enough memory was available.
     */
   explicit HyperException(hyper_error_t* error);

   /**
    * Constructs a `hyperapi::HyperException` from a `hyper_error_t` that represents a cause.
    * The `root_error` will be kept alive for the lifetime of this object.
    *
    * \param error  The error.
    * \param top_level_error  The top level error that has to be kept alive since it manages the lifetime of `error`.
    */
   explicit HyperException(hyper_error_t* error, const std::shared_ptr<const hyper_error_t> top_level_error);

   /**
     * Returns the string value of the field.
     * \param key The key to the error field.
     * \return The field value.
     */
   const char* getStringField(hyper_error_field_key key) const noexcept;

   /**
     * Returns the integer value of the field.
     * \param key The key to the error field.
     * \return The field value.
     */
   int getIntField(hyper_error_field_key key) const noexcept;

   /**
     * Returns the unsigned integer value of the field.
     * \param key The key to the error field.
     * \return The field value.
     */
   uint32_t getUIntField(hyper_error_field_key key) const noexcept;

   /**
     * Returns the pointer to the cause.
     */
   hyper_error_t* getCauseHandle() const noexcept;

   /// The error handle (the error is assumed to be immutable).
   std::shared_ptr<const hyper_error_t> m_error;

   /// Storage for the string representation.
   mutable std::string stringRepresentation;

   friend const char* internal::getSqlState(const hyperapi::HyperException&);
   friend const char* internal::getDetail(const hyperapi::HyperException&);
   friend int internal::getErrorCode(const hyperapi::HyperException&);
   friend int internal::getErrorCategory(const hyperapi::HyperException&);
   friend HyperException internal::makeHyperException(const std::string&, const std::string&, ContextId);
   friend HyperException internal::makeHyperException(hyper_error_t*);
   friend class HyperExceptionTest;
};
}

#include <hyperapi/impl/HyperException.impl.hpp>

#endif
