// -----------------------------------------------------------------------------
//
// This file is the copyrighted property of Tableau Software and is protected
// by registered patents and other applicable U.S. and international laws and
// regulations.
//
// Unlicensed use of the contents of this file is prohibited. Please refer to
// the NOTICES.txt file for further details.
//
// -----------------------------------------------------------------------------

#ifndef TABLEAU_HYPER_IMPL_INFRA_HPP
#define TABLEAU_HYPER_IMPL_INFRA_HPP

#include <atomic>
#include <cassert>
#include <cstring>
#include <hyperapi/optional.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <hyperapi/hyperapi.h>

namespace hyperapi {

class Connection;
class HyperException;
class HyperProcess;
class Result;

namespace internal {

// ----------------------------------------------------------------------------

#ifdef __has_builtin
#if __has_builtin(__builtin_unreachable)
#define HYPER_UNREACHABLE() __builtin_unreachable()
#endif
#endif

#ifndef HYPER_UNREACHABLE
#if defined(__GNUC__)
#define HYPER_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define HYPER_UNREACHABLE() __assume(false)
#else
#define HYPER_UNREACHABLE() ((void) 0)
#endif
#endif

// Deprecation Macro
#if (__cplusplus >= 201402L)
#define HYPER_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(__GNUC__) || defined(__clang__)
#define HYPER_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#define HYPER_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#define HYPER_DEPRECATED(msg)
#endif
// ----------------------------------------------------------------------------

inline std::atomic<bool>& abortInPrecondition() {
   static std::atomic<bool> value(true);
   return value;
}

inline void setAbortInPrecondition(bool value) {
   abortInPrecondition() = value;
}

/** A TypeTag for the unknown type. Only supported for insertion of null values. */
struct AnyType {
   AnyType() = delete;
};

/** Surrogate for C++14 `std::exchange()`. */
template <class T, class U>
inline T exchange(T& t, U&& u) noexcept {
   T old = std::move(t);
   t = std::forward<U>(u);
   return old;
}

/** Is a C-string empty? */
inline bool isCStrEmptyOrNull(const char* c) noexcept {
   return (c == nullptr) || (*c == '\0');
}

/**
 * Throws a std::logic_error with the appropriate error message for precondition violations.
 * \param expression  The expression that was violated.
 * \param file  The file in which the precondition is located.
 * \param line  The line inside the file where the precondition is located.
 * \param function  The name of the function in which the precondition is located.
 * \param message  An optional message that will be put into the error.
 */
template <typename T>
inline void onPreconditionViolation(const char* expression, const char* file, int line, const char* function, T message) {
   std::ostringstream ss;
   ss << "Precondition violation in `" << function << "()`: " << message << '\n'
      << "Precondition: `(" << expression << ")`\nLocation: " << file << ':' << line;
   if (hyperapi::internal::abortInPrecondition()) {
      std::cerr << ss.str() << std::endl;
      std::abort();
   }
   throw std::logic_error(ss.str());
}

/// Check the given precondition exception. Throws a descriptive
/// std::logic_error on failure containing also the given message.
#define HYPER_PRECONDITION_MSG(expr, message) \
   (static_cast<bool>(expr) ? void(0) : hyperapi::internal::onPreconditionViolation(#expr, __FILE__, __LINE__, __func__, message))

/// Check the given precondition exception. Throws a descriptive
/// std::logic_error on failure.
#define HYPER_PRECONDITION(expr) HYPER_PRECONDITION_MSG(expr, "")

// Those forward declarations are here s.t. they do not clutter the non-internal headers.
class HyperTableDefinition;
hyper_connection_t* getHandle(hyperapi::Connection&) noexcept;
Result executePreparedQuery(hyperapi::Connection&, const std::string&, hyper_rowset_result_format_t);
Result executeQueryParams(hyperapi::Connection&, const std::string&, hyper_rowset_result_format_t);
const char* getSqlState(const hyperapi::HyperException&);
const char* getDetail(const hyperapi::HyperException&);
int getErrorCode(const hyperapi::HyperException&);
int getErrorCategory(const hyperapi::HyperException&);
hyperapi::optional<int> getExitCode(hyperapi::HyperProcess&);
HyperException makeHyperException(hyper_error_t*);
template <class ReturnType>
struct ValueAccess;
struct ValueInserter;
}
}
#endif
