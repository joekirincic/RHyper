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

#ifndef TABLEAU_HYPER_TIMESTAMP_HPP
#define TABLEAU_HYPER_TIMESTAMP_HPP

#include <hyperapi/Date.hpp>
#include <hyperapi/Time.hpp>
#include <string>

namespace hyperapi {
/**
  * A timestamp data value.
  */
class Timestamp final {
   public:
   /**
     * Default constructor.
     */
   Timestamp() noexcept {}

   /**
     * Creates a timestamp value with a date and a time component.
     *
     * \param date The date component.
     * \param time The time component.
     */
   Timestamp(Date date, Time time) noexcept;

   /**
     * Gets the date component of the timestamp.
     *
     * \return The date component.
     */
   const Date& getDate() const noexcept;

   /**
     * Gets the time component of the timestamp.
     *
     * \return The time component.
     */
   const Time& getTime() const noexcept;

   /**
     * Returns a string representation for debugging.
     *
     * The returned string is in the format `<date> <time>`, where `<date>` is formatted as `±YYYY-MM-DD` and `<time>` as `HH:MM:SS.ssssss`.
     */
   std::string toString() const;

   /**
     * Equality operator.
     */
   friend bool operator==(const Timestamp& lhs, const Timestamp& rhs) noexcept;
   /**
     * Greater operator.
     */
   friend bool operator>(const Timestamp& lhs, const Timestamp& rhs) noexcept;
   /**
     * Not equal operator.
     */
   friend bool operator!=(const Timestamp& a, const Timestamp& b) noexcept { return !(a == b); }
   /**
     * Less than operator.
     */
   friend bool operator<(const Timestamp& a, const Timestamp& b) noexcept { return (b > a); }
   /**
     * Less than or equal operator.
     */
   friend bool operator<=(const Timestamp& a, const Timestamp& b) noexcept { return !(a > b); }
   /**
     * Greater or equal operator.
     */
   friend bool operator>=(const Timestamp& a, const Timestamp& b) noexcept { return !(a < b); }

   /** Stream output operator */
   friend std::ostream& operator<<(std::ostream& os, const Timestamp& obj) { return os << obj.toString(); }

   private:
   friend class Inserter;
   friend class Value;
   friend struct std::hash<hyperapi::Timestamp>;
   friend struct internal::ValueInserter;

   struct raw_t {};

   /**
     * The number of microseconds per day.
     */
   static constexpr uint64_t microseconds_per_day = 24ull * 60 * 60 * 1000 * 1000;

   /**
     * Creates a timestamp from a raw timestamp value encoded as microseconds since 1 January 4713 BC.
     *
     * \param rawTimestamp  The raw timestamp value.
     */
   explicit Timestamp(hyper_timestamp_t rawTimestamp, raw_t) noexcept;

   /**
     * Gets the raw timestamp value encoded as microseconds since 1 January 4713 BC.
     *
     * \return The raw timestamp value.
     */
   hyper_timestamp_t getRaw() const noexcept;

   /**
     * The raw timestamp value encoded as microseconds since 1 January 4713 BC.
     */
   hyper_timestamp_t m_representation = 0;
   /**
     * The date component of the raw timestamp value.
     */
   Date m_date;
   /**
     * The time component of the raw timestamp value.
     */
   Time m_time;
};
}

#include <hyperapi/impl/Timestamp.impl.hpp>

#endif
