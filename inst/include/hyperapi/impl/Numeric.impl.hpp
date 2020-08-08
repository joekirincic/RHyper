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

#ifndef TABLEAU_HYPER_IMPL_NUMERIC_IMPL_HPP
#define TABLEAU_HYPER_IMPL_NUMERIC_IMPL_HPP

#include <hyperapi/HyperException.hpp>
#include <hyperapi/hyperapi.h>

#include <cassert>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>

namespace hyperapi {

namespace internal {

static int64_t tenPow[]{1ll,
                        10ll,
                        100ll,
                        1000ll,
                        10000ll,
                        100000ll,
                        1000000ll,
                        10000000ll,
                        100000000ll,
                        1000000000ll,
                        10000000000ll,
                        100000000000ll,
                        1000000000000ll,
                        10000000000000ll,
                        100000000000000ll,
                        1000000000000000ll,
                        10000000000000000ll,
                        100000000000000000ll,
                        1000000000000000000ll};

inline std::string numericToString(int64_t svalue, unsigned scale) {
   if (scale == 0) {
      return std::to_string(svalue);
   }

   char buffer[30];
   char* writer = buffer + sizeof(buffer);
   *(--writer) = 0;

   // Check sign
   bool neg = false;
   uint64_t value;
   if (svalue < 0) {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4146)
#endif
      // Cast to unsigned before negating to avoid potential undefined behavior from negating INT64_MIN.
      value = -static_cast<uint64_t>(svalue);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
      neg = true;
   } else {
      value = static_cast<uint64_t>(svalue);
   }

   // Write the raw value
   unsigned digitsLeft = scale;
   if (!value) {
      *(--writer) = '0';
      if (digitsLeft) {
         if (digitsLeft == 1)
            *(--writer) = '.';
         --digitsLeft;
      }
   } else
      while (value) {
         *(--writer) = '0' + (value % 10);
         value /= 10;
         if (digitsLeft) {
            if (digitsLeft == 1)
               *(--writer) = '.';
            --digitsLeft;
         }
      }

   // Pad if needed
   if (digitsLeft) {
      for (; digitsLeft; --digitsLeft)
         *(--writer) = '0';
      *(--writer) = '.';
   }
   if ((*writer) == '.')
      *(--writer) = '0';
   if (neg)
      *(--writer) = '-';

   // Output
   unsigned len = static_cast<uint32_t>((buffer + sizeof(buffer) - 1) - writer);
   return std::string(writer, len);
}

template <typename T1, typename T2>
[[noreturn]] void throwOutOfRange(T1 value, T2 limit, unsigned precision, unsigned scale, const char* text) {
   std::stringstream ss;
   ss << "Value " << value << text << limit << " for type NUMERIC(" << precision << "," << scale << ")";
   throw internal::makeHyperException(ss.str(), "", ContextId(0x4b4091a3u));
}

template <typename T>
inline int64_t scaleInt(unsigned scale, unsigned scaleAdjustment, unsigned precision, T value) {
   constexpr bool isSigned = T(0) > T(-1);
   int64_t minVal = std::numeric_limits<int64_t>::min() / internal::tenPow[scale];
   int64_t maxVal = std::numeric_limits<int64_t>::max() / internal::tenPow[scale];
   if (isSigned) {
      if ((std::numeric_limits<T>::max() > maxVal) && (static_cast<int64_t>(value) > maxVal)) {
         throwOutOfRange(
            numericToString(static_cast<int64_t>(value), scaleAdjustment), numericToString(maxVal, scaleAdjustment), precision, scale + scaleAdjustment,
            " is above the maximum value of ");
      }
      if ((static_cast<int64_t>(std::numeric_limits<T>::min()) < minVal) && (static_cast<int64_t>(value) < minVal)) {
         throwOutOfRange(
            numericToString(static_cast<int64_t>(value), scaleAdjustment), numericToString(minVal, scaleAdjustment), precision, scale + scaleAdjustment,
            " is below the minimum value of ");
      }
   } else {
      if ((static_cast<uint64_t>(std::numeric_limits<T>::max()) > static_cast<uint64_t>(maxVal)) && (static_cast<uint64_t>(value) > static_cast<uint64_t>(maxVal))) {
         throwOutOfRange(
            static_cast<uint64_t>(value), numericToString(maxVal, scaleAdjustment), precision, scale + scaleAdjustment, " is above the maximum value of ");
      }
   }
   return static_cast<int64_t>(value) * tenPow[scale];
}

template <typename T>
inline int64_t scaleInt(unsigned scale, unsigned precision, T value) {
   return scaleInt(scale, 0u, precision, value);
}

inline constexpr unsigned subtractAndClampAtZero(unsigned a, unsigned b) {
   return (static_cast<int>(a) - static_cast<int>(b) < 0) ? 0 : a - b;
}

inline int64_t cast(int64_t rawNumeric1, unsigned precision1, unsigned scale1, unsigned /*precision2*/, unsigned scale2) {
   if (scale2 < scale1) {
      return internal::scaleInt(internal::subtractAndClampAtZero(scale1, scale2), scale2, precision1, rawNumeric1);
   } else {
      return rawNumeric1 / internal::tenPow[internal::subtractAndClampAtZero(scale2, scale1)];
   }
}
}

template <unsigned p, unsigned s>
Numeric<p, s>::Numeric(short value)
   : m_value(internal::scaleInt(s, p, value)) {
}

template <unsigned p, unsigned s>
Numeric<p, s>::Numeric(unsigned short value)
   : m_value(internal::scaleInt(s, p, value)) {
}

template <unsigned p, unsigned s>
Numeric<p, s>::Numeric(int value)
   : m_value(internal::scaleInt(s, p, value)) {
}

template <unsigned p, unsigned s>
Numeric<p, s>::Numeric(unsigned int value)
   : m_value(internal::scaleInt(s, p, value)) {
}

template <unsigned p, unsigned s>
Numeric<p, s>::Numeric(long value)
   : m_value(internal::scaleInt(s, p, value)) {
}

template <unsigned p, unsigned s>
Numeric<p, s>::Numeric(unsigned long value)
   : m_value(internal::scaleInt(s, p, value)) {
}

template <unsigned p, unsigned s>
Numeric<p, s>::Numeric(long long value)
   : m_value(internal::scaleInt(s, p, value)) {
}

template <unsigned p, unsigned s>
Numeric<p, s>::Numeric(unsigned long long value)
   : m_value(internal::scaleInt(s, p, value)) {
}

template <unsigned p, unsigned s>
Numeric<p, s>::Numeric(double value)
   : m_value(static_cast<int64_t>(round(value * static_cast<double>(internal::tenPow[s])))) {
   double max = static_cast<double>(std::numeric_limits<int64_t>::max() / internal::tenPow[s]);
   double min = static_cast<double>(std::numeric_limits<int64_t>::min() / internal::tenPow[s]);
   if (!(value <= max)) { // Negation used to handle NaN correctly
      internal::throwOutOfRange(value, max, p, s, " is above the maximum value of ");
   }
   if (value < min) {
      internal::throwOutOfRange(value, min, p, s, " is above the maximum value of ");
   }
}

template <unsigned p, unsigned s>
Numeric<p, s>::Numeric(float value)
   : Numeric(static_cast<double>(value)) {
}

template <unsigned p, unsigned s>
Numeric<p, s>::Numeric(long double value)
   : Numeric(static_cast<double>(value)) {
}

template <unsigned p, unsigned s>
Numeric<p, s>::Numeric(const hyperapi::string_view value)
   : m_value() {
   const char* begin = value.data();
   const char* end = begin + value.size();
   hyper_error_t* error = hyper_parse_numeric(&begin, end, p, s, &m_value);
   if (error) {
      throw internal::makeHyperException(error);
   }
}

template <unsigned p, unsigned s>
double Numeric<p, s>::doubleValue() const noexcept {
   return static_cast<double>(m_value) / internal::tenPow[s];
}

template <unsigned p, unsigned s>
int64_t Numeric<p, s>::intValue() const noexcept {
   return m_value / internal::tenPow[s];
}

template <unsigned precision, unsigned scale>
template <unsigned otherPrecision, unsigned otherScale>
Numeric<precision, scale>::Numeric(hyperapi::Numeric<otherPrecision, otherScale> other) : Numeric(other.m_value, otherPrecision, otherScale, raw_t()) {}

template <unsigned precision, unsigned scale>
inline Numeric<precision, scale>::Numeric(int64_t rawNumeric, unsigned otherPrecision, unsigned otherScale, raw_t) {
   m_value = internal::cast(rawNumeric, precision, scale, otherPrecision, otherScale);
}

template <unsigned p, unsigned s>
std::string Numeric<p, s>::stringValue() const {
   return internal::numericToString(m_value, s);
}

template <unsigned p, unsigned s>
std::string Numeric<p, s>::toString() const {
   return stringValue();
}
}

#endif
