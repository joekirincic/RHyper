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
#ifndef TABLEAU_HYPER_IMPL_OPTIONAL_IMPL_HPP
#define TABLEAU_HYPER_IMPL_OPTIONAL_IMPL_HPP

#include <hyperapi/optional.hpp>
#include <utility>

namespace hyperapi {
#ifdef hyper_use_own_optional

template <typename T>
inline optional<T>::optional(const optional& other) {
   if (other.exists) {
      create(*other);
   }
}

template <typename T>
inline optional<T>::optional(optional&& other) {
   if (other.exists) {
      create(std::move(*other));
   }
}

template <typename T>
template <typename U, typename>
optional<T>::optional(U&& value) {
   create(std::forward<U>(value));
}

template <typename T>
inline optional<T>& optional<T>::operator=(const optional& other) {
   if (exists) {
      if (other.exists) {
         **this = *other;
      } else {
         reset();
      }
   } else {
      if (other.exists) {
         create(*other);
      }
   }
   return *this;
}

template <typename T>
inline optional<T>& optional<T>::operator=(optional&& other) {
   if (exists) {
      if (other.exists) {
         **this = std::move(*other);
      } else {
         reset();
      }
   } else {
      if (other.exists) {
         create(std::move(*other));
      }
   }
   return *this;
}

template <typename T>
void optional<T>::swap(optional& other) {
   if (this == &other) {
      return;
   }
   if (exists) {
      if (other.exists) {
         using std::swap;
         swap(**this, *other);
      } else {
         other.create(std::move(**this));
         reset();
      }
   } else {
      if (other.exists) {
         create(std::move(*other));
         other.reset();
      }
   }
}

template <typename T>
inline void optional<T>::reset() noexcept {
   if (exists) {
      reinterpret_cast<T*>(data)->~T();
      exists = false;
   }
}

template <typename T>
template <typename... Args>
void optional<T>::emplace(Args&&... args) {
   reset();
   create(std::forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
inline void optional<T>::create(Args&&... args) {
   new (data) T(std::forward<Args>(args)...);
   exists = true;
}
#endif
}
#endif
