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

#ifndef TABLEAU_HYPER_BYTESPAN_HPP
#define TABLEAU_HYPER_BYTESPAN_HPP

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace hyperapi {

/**
 * An arbitrarily-sized binary value.
 */
struct ByteSpan {
   /** The start of the binary data */
   const uint8_t* data;
   /** The size of the data */
   size_t size;

   /** Constructor */
   ByteSpan(const uint8_t* data, size_t size) noexcept : data(data), size(size) {}

   /** Constructor */
   ByteSpan(const std::vector<uint8_t>& bytes) noexcept : data(bytes.data()), size(bytes.size()) {}

   /**
    * Equality operator.
    */
   friend bool operator==(const ByteSpan& lhs, const ByteSpan& rhs) noexcept;
   /**
    * Greater operator.
    */
   friend bool operator>(const ByteSpan& lhs, const ByteSpan& rhs) noexcept;
   /**
    * Not equal operator.
    */
   friend bool operator!=(const ByteSpan& a, const ByteSpan& b) noexcept { return !(a == b); }
   /**
    * Less than operator.
    */
   friend bool operator<(const ByteSpan& a, const ByteSpan& b) noexcept { return (b > a); }
   /**
    * Less than or equal operator.
    */
   friend bool operator<=(const ByteSpan& a, const ByteSpan& b) noexcept { return !(a > b); }
   /**
    * Greater or equal operator.
    */
   friend bool operator>=(const ByteSpan& a, const ByteSpan& b) noexcept { return !(a < b); }

   /**
    * Returns a string representation of the binary data.
    * The binary data is printed byte-wise. All printable bytes (according to std::isprint()) are
    * printed as is. All other bytes are printed as escaped hexadecimal characters (e.g.: `\\x04`).
    * \return a string representation of the binary data.
    */
   std::string toString() const;

   /** Stream output operator */
   friend std::ostream& operator<<(std::ostream& os, const ByteSpan& obj) { return os << obj.toString(); }
};
}

namespace std {
/** Specialization of `std::hash` for `hyperapi::ByteSpan`. */
template <>
struct hash<hyperapi::ByteSpan> {
   /** Calculates the hash value of the given byte span. */
   size_t operator()(const hyperapi::ByteSpan&) const noexcept;
};
}

#include <hyperapi/impl/ByteSpan.impl.hpp>

#endif
