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

#ifndef TABLEAU_HYPER_STRING_VIEW_HPP
#define TABLEAU_HYPER_STRING_VIEW_HPP

#if !defined(__cplusplus) || (__cplusplus < 201703L) /* Use own string view if C++ is older than C++17 */
#define hyper_use_own_string_view
#endif

#ifdef hyper_use_own_string_view
#include <cstring>
#include <ostream>
#include <string>
#else
#include <string_view>
#endif

namespace hyperapi {
#ifdef hyper_use_own_string_view
/**
 * Describes an object that can refer to a constant, contiguous sequence of char-like objects.
 * Surrogate for C++17 `std::string_view`
 */
class string_view final {
   public:
   /// Constructor.
   string_view(const char* data, size_t length) noexcept
      : m_data(data), m_length(length) {
   }

   /// Constructor, implicit conversion from a null terminated character array.
   string_view(const char* c)
      : m_data(c), m_length(std::strlen(c)) {
   }

   /// Constructor, implicit conversion from a std::string.
   string_view(const std::string& s)
      : m_data(s.data()), m_length(s.size()) {
   }

   /**
     * Returns the number of elements in the view.
     */
   size_t size() const noexcept { return m_length; }
   /**
     * Returns a pointer to the underlying character array. The pointer is such that the range [data(); data() + size())
     * is valid and the values in it correspond to the values of the view.
     *
     * Note that this character array is not guaranteed to be null-terminated.
     *
     * \return A pointer to the underlying character array.
     */
   const char* data() const noexcept { return m_data; }

   /**
     * Compares two character sequences.
     *
     * \return A negative value if this view is less than the other character sequence; zero if both character
     * sequences are equal; a positive value if this view is greater than the other character sequence.
     */
   int compare(const string_view& other) const noexcept;

   /**
     * Implicit conversion to std::string.
     */
   operator std::string() const { return std::string(data(), size()); }

   /**
     * Equality operator.
     */
   friend bool operator==(const string_view& a, const string_view& b) noexcept { return a.compare(b) == 0; }
   /**
     * Greater operator.
     */
   friend bool operator>(const string_view& a, const string_view& b) noexcept { return a.compare(b) > 0; }
   /**
     * Not equal operator.
     */
   friend bool operator!=(const string_view& a, const string_view& b) noexcept { return !(a == b); }
   /**
     * Smaller operator.
     */
   friend bool operator<(const string_view& a, const string_view& b) noexcept { return (b > a); }
   /**
     * Smaller or equal operator.
     */
   friend bool operator<=(const string_view& a, const string_view& b) noexcept { return (a == b) || (a < b); }
   /**
     * Greater or equal operator.
     */
   friend bool operator>=(const string_view& a, const string_view& b) noexcept { return (a == b) || (a > b); }

   /** Stream output operator */
   friend std::ostream& operator<<(std::ostream& os, const string_view& v) { return os.write(v.data(), static_cast<long>(v.size())); }

   /** Returns whether the view is empty. */
   bool empty() { return m_length == 0; }

   private:
   /// The underlying character array.
   const char* m_data = nullptr;

   /// The length of the underlying character array.
   size_t m_length = 0;
};
}

namespace std {

/** Specialization of `std::hash` for `hyperapi::string_view`. */
template <>
struct hash<hyperapi::string_view> {
   /** Calculates the hash value of the given string view. */
   size_t operator()(const hyperapi::string_view& sv) const noexcept;
};
}
#else /* C++17 or greater */
using string_view = std::string_view;
}
#endif

#ifdef hyper_use_own_string_view
#include <hyperapi/impl/string_view.impl.hpp>
#endif

#endif
