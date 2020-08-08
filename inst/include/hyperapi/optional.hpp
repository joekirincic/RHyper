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

#ifndef TABLEAU_HYPER_OPTIONAL_HPP
#define TABLEAU_HYPER_OPTIONAL_HPP

#if !defined(__cplusplus) || (__cplusplus < 201703L) /* Use own optional if C++ is older than C++17 */
#define hyper_use_own_optional
#endif

#ifdef hyper_use_own_optional
#include <exception>
#include <type_traits>
#include <utility>
#else
#include <optional>
#endif

#ifdef hyper_use_own_optional
// Our optional surrogate does not support `constexpr`
#define CONSTEXPR_OPTIONAL const
#else
#define CONSTEXPR_OPTIONAL constexpr
#endif

namespace hyperapi {
#ifndef hyper_use_own_optional /* C++17 or greater */
template <typename T>
using optional = std::optional<T>;
using bad_optional_access = std::bad_optional_access;
#else
/** Surrogate for C++17 `std::bad_optional_access` */
class bad_optional_access : public std::exception {
   using std::exception::exception;
};

/** Surrogate for C++17 `std::optional` */
template <typename T>
class optional {
   private:
   /// The data space
   alignas(T) char data[sizeof(T)];

   /// Does the object currently exist?
   bool exists = false;

   public:
   /** The value type */
   using value_type = T;

   /** Constructor */
   optional() noexcept = default;

   /** Copy constructor */
   optional(const optional& other);

   /** Move constructor */
   optional(optional&& other);

   /** Constructor */
   template <
      typename U = value_type,
      typename = typename std::enable_if<!std::is_same<typename std::decay<U>::type, optional<T>>::value>::type>
   optional(U&& value);

   /** Copy assignment */
   optional& operator=(const optional& other);

   /** Move assignment */
   optional& operator=(optional&& other);

   /** Comparison */
   template <class Other>
   bool operator==(const optional<Other>& other) const noexcept {
      return (exists == other.exists) && ((!exists) || (**this == *other));
   }
   /** Comparison */
   template <class Other>
   bool operator!=(const optional<Other>& other) const noexcept { return !(*this == other); }

   /** Destructor */
   ~optional() { reset(); }

   /** Checks whether `*this` contains a value. */
   bool has_value() const noexcept { return exists; }
   /** Checks whether `*this` contains a value. */
   explicit operator bool() const noexcept { return exists; }

   /** Value access */
   T& value() & {
      if (!*this) {
         throw bad_optional_access{};
      }
      return **this;
   }
   /** Value access */
   const T& value() const& {
      if (!*this) {
         throw bad_optional_access{};
      }
      return **this;
   }
   /** Value access */
   T&& value() && {
      if (!*this) {
         throw bad_optional_access{};
      }
      return **this;
   }
   /** Value access */
   const T&& value() const&& {
      if (!*this) {
         throw bad_optional_access{};
      }
      return **this;
   }

   /** Value or default */
   template <class U>
   T value_or(U&& default_value) const& {
      return bool(*this) ? **this : static_cast<T>(std::forward<U>(default_value));
   }

   /** Value or default */
   template <class U>
   T value_or(U&& default_value) && {
      return bool(*this) ? std::move(**this) : static_cast<T>(std::forward<U>(default_value));
   }

   /** Value access */
   T* operator->() noexcept { return ptr(); }
   /** Value access */
   const T* operator->() const noexcept { return ptr(); }

   /** Value access */
   const T& operator*() const& { return *ptr(); }
   /** Value access */
   T& operator*() & { return *ptr(); }
   /** Value access */
   const T&& operator*() const&& { return *ptr(); }
   /** Value access */
   T&& operator*() && { return std::move(*ptr()); }

   /** Reset */
   void reset() noexcept;
   /** Swap */
   void swap(optional& other);
   /** Emplace */
   template <typename... Args>
   void emplace(Args&&... args);

   private:
   /// Create
   template <typename... Args>
   void create(Args&&... args);

   /// Access
   T* ptr() noexcept { return exists ? reinterpret_cast<T*>(data) : nullptr; }
   /// Access
   const T* ptr() const noexcept { return exists ? reinterpret_cast<const T*>(data) : nullptr; }
};

#endif
}
#include <hyperapi/impl/optional.impl.hpp>
#endif
