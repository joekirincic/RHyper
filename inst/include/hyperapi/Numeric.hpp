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

#ifndef TABLEAU_HYPER_NUMERIC_HPP
#define TABLEAU_HYPER_NUMERIC_HPP

#include <cstdint>
#include <hyperapi/SqlType.hpp>
#include <hyperapi/string_view.hpp>
#include <string>
#include <hyperapi/hyperapi.h>

class Numeric_rawValue_Test;

namespace hyperapi {

/**
 * A fixed-point numeric data value with `scale` fraction digits and `precision` digits overall.
 *
 * Note that this class stores a fixed-point value as a 64-bit integer and leniently accepts all values
 * that fit into 64-bit, regardless of whether they would actually fit into `precision` digits.
 * Thus, you can create, e.g., a Numeric<2,2>(1000.35). The reason for allowing this is that Hyper
 * does the same. However, it is not guaranteed that such values can be inserted into a table.
 */
template <unsigned precision_value, unsigned scale_value>
class Numeric final {
   public:
   static constexpr unsigned precision = precision_value;
   static constexpr unsigned scale = scale_value;
   static_assert(precision >= scale, "The precision of a numeric must be greater or equal than the scale");
   static_assert(precision < 19, "Precision must be less than 19");

   /**
     * Default constructor.
     */
   Numeric() noexcept {}

   /**
     * Creates a numeric value from an integer.
     * \throws HyperException  if the value cannot be represented by a 64-bit Numeric.
     */
   Numeric(short value);
   /**
     * Creates a numeric value from an integer.
     * \throws HyperException  if the value cannot be represented by a 64-bit Numeric.
     */
   Numeric(unsigned short value);
   /**
     * Creates a numeric value from an integer.
     * \throws HyperException  if the value cannot be represented by a 64-bit Numeric.
     */
   Numeric(int value);
   /**
     * Creates a numeric value from an integer.
     * \throws HyperException  if the value cannot be represented by a 64-bit Numeric.
     */
   Numeric(unsigned value);
   /**
     * Creates a numeric value from an integer.
     * \throws HyperException  if the value cannot be represented by a 64-bit Numeric.
     */
   Numeric(long value);
   /**
     * Creates a numeric value from an integer.
     * \throws HyperException  if the value cannot be represented by a 64-bit Numeric.
     */
   Numeric(unsigned long value);
   /**
     * Creates a numeric value from an integer.
     * \throws HyperException  if the value cannot be represented by a 64-bit Numeric.
     */
   Numeric(long long value);
   /**
     * Creates a numeric value from an integer.
     * \throws HyperException  if the value cannot be represented by a 64-bit Numeric.
     */
   Numeric(unsigned long long value);
   /**
     * Creates a numeric value from a double; may lose accuracy.
     * \throws HyperException  if the value cannot be represented by a 64-bit Numeric due to overflow.
     */
   Numeric(float value);
   /**
     * Creates a numeric value from a double; may lose accuracy.
     * \throws HyperException  if the value cannot be represented by a 64-bit Numeric due to overflow.
     */
   Numeric(double value);
   /**
     * Creates a numeric value from a double; may lose accuracy.
     * \throws HyperException  if the value cannot be represented by a 64-bit Numeric due to overflow.
     */
   Numeric(long double value);

   /**
     * Creates a numeric value from another numeric value with different precision and scale.
     * \throws HyperException  if the value cannot be represented by a 64-bit Numeric.
     */
   template <unsigned otherPrecision, unsigned otherScale>
   explicit Numeric(hyperapi::Numeric<otherPrecision, otherScale> other);

   /**
     * Creates a numeric value from a string representation.
     * \throws HyperException  if `value` is not a valid numeric representation or overflows.
     */
   explicit Numeric(const hyperapi::string_view value);

   /**
     * Gets an exact string representation, which is round-trip compatible with the constructor,
     * i.e., `n == Numeric(n.stringValue())`
     */
   std::string stringValue() const;

   /**
     * Gets an integer representation of this value; if the value has fraction digits, these will be truncated.
     */
   int64_t intValue() const noexcept;
   /**
     * Explicit conversion to int64_t
     */
   operator int64_t() const noexcept { return intValue(); }

   /**
     * Gets a double representation of this value; may lose accuracy
     */
   double doubleValue() const noexcept;
   /**
     * Explicit conversion to double
     */
   operator double() const noexcept { return doubleValue(); }

   /**
     * Gets a string representation for debugging.
     * Currently, the result is equivalent to calling stringValue(), but this is not guaranteed; it might change in future versions.
     */
   std::string toString() const;

   /**
     * Equality operator.
     */
   friend bool operator==(const Numeric& a, const Numeric& b) noexcept { return a.m_value == b.m_value; }
   /**
     * Greater operator.
     */
   friend bool operator>(const Numeric& a, const Numeric& b) noexcept { return a.m_value > b.m_value; }
   /**
     * Not equal operator.
     */
   friend bool operator!=(const Numeric& a, const Numeric& b) noexcept { return !(a == b); }
   /**
     * Less than operator.
     */
   friend bool operator<(const Numeric& a, const Numeric& b) noexcept { return (b > a); }
   /**
     * Less than or equal operator.
     */
   friend bool operator<=(const Numeric& a, const Numeric& b) noexcept { return !(a > b); }
   /**
     * Greater or equal operator.
     */
   friend bool operator>=(const Numeric& a, const Numeric& b) noexcept { return !(a < b); }

   /** Stream output operator */
   friend std::ostream& operator<<(std::ostream& os, const Numeric& obj) { return os << obj.toString(); }

   private:
   friend class ::Numeric_rawValue_Test;
   template <unsigned otherPrecision, unsigned otherScale>
   friend class Numeric;
   friend class Inserter;
   template <class ReturnType>
   friend struct internal::ValueAccess;
   friend struct internal::ValueInserter;

   struct raw_t {};

   /**
     * Creates a Numeric value from the raw bit representation
     *
     * \param rawNumeric The raw numeric value.
     */
   explicit Numeric(int64_t rawNumeric, raw_t) noexcept
      : m_value(rawNumeric) {
   }

   /**
     * Creates a Numeric value from the raw bit representation and
     * dynamic precicision and scale parameters that may differ from this Numeric's precision and scale.
     *
     * \param rawNumeric The raw numeric value.
     * \param otherPrecision The precision of the raw value.
     * \param otherScale The scale of the raw value.
     */
   explicit Numeric(int64_t rawNumeric, unsigned otherPrecision, unsigned otherScale, raw_t);

   /**
     * The encoding of this numeric
     */
   int64_t m_value = 0;
};
}

namespace std {

/** Specialization of `std::hash` for `hyperapi::Numeric`. */
template <unsigned p, unsigned s>
struct hash<hyperapi::Numeric<p, s>> {
   /** Calculates the hash value of the given interval. */
   size_t operator()(hyperapi::Numeric<p, s> n) const noexcept { return std::hash<int64_t>()(n.m_value); }
};
}

#include <hyperapi/impl/Numeric.impl.hpp>

#endif
