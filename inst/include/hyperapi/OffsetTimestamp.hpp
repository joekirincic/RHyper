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

#ifndef TABLEAU_HYPER_OFFSET_TIMESTAMP_HPP
#define TABLEAU_HYPER_OFFSET_TIMESTAMP_HPP

#include <chrono>
#include <hyperapi/Date.hpp>
#include <hyperapi/Time.hpp>
#include <string>

namespace hyperapi {
/**
  * A timestamp data value with an offset to UTC.
  */
class OffsetTimestamp final {
   public:
   /**
     * Default constructor.
     */
   OffsetTimestamp() noexcept {}

   /**
     * Creates a timestamp value with a date, time, and offset component.
     *
     * \param date The date component.
     * \param time The time component.
     * \param offset The offset to UTC in minutes.
     */
   OffsetTimestamp(Date date, Time time, std::chrono::minutes offset) noexcept;

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
     * Gets the offset to UTC of the timestamp in minutes.
     *
     * \return The offset.
     */
   std::chrono::minutes getOffset() const noexcept;

   /**
     * Returns a string representation for debugging.
     *
     * The returned string is in the format `<date> <time><offset>`, where `<date>` is formatted as `±YYYY-MM-DD`, `<time>` as `HH:MM:SS.ssssss`, and offset as `±HH:MM`.
     */
   std::string toString() const;

   /**
     * Equality operator.
     */
   friend bool operator==(const OffsetTimestamp& lhs, const OffsetTimestamp& rhs) noexcept;
   /**
     * Greater operator.
     */
   friend bool operator>(const OffsetTimestamp& lhs, const OffsetTimestamp& rhs) noexcept;
   /**
     * Not equal operator.
     */
   friend bool operator!=(const OffsetTimestamp& a, const OffsetTimestamp& b) noexcept { return !(a == b); }
   /**
     * Less than operator.
     */
   friend bool operator<(const OffsetTimestamp& a, const OffsetTimestamp& b) noexcept { return (b > a); }
   /**
     * Less than or equal operator.
     */
   friend bool operator<=(const OffsetTimestamp& a, const OffsetTimestamp& b) noexcept { return !(a > b); }
   /**
     * Greater or equal operator.
     */
   friend bool operator>=(const OffsetTimestamp& a, const OffsetTimestamp& b) noexcept { return !(a < b); }

   /** Stream output operator */
   friend std::ostream& operator<<(std::ostream& os, const OffsetTimestamp& obj) { return os << obj.toString(); }

   private:
   friend class Inserter;
   friend class Value;
   friend struct std::hash<hyperapi::OffsetTimestamp>;
   friend struct internal::ValueInserter;

   struct raw_t {};

   /**
     * The number of microseconds per minute.
     */
   static constexpr uint64_t microseconds_per_minute = 60 * 1000 * 1000;
   /**
     * The number of microseconds per day.
     */
   static constexpr uint64_t microseconds_per_day = 24ull * 60 * microseconds_per_minute;

   /**
     * Creates a timestamp from a raw timestamp value encoded as microseconds since 1 January 4713 BC.
     *
     * \param rawTimestamp  The raw timestamp value.
     */
   explicit OffsetTimestamp(hyper_timestamp_t rawTimestamp, raw_t) noexcept;

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
   /**
     * The offset in minutes to UTC of the raw timestamp value.
     */
   std::chrono::minutes m_offset{0};
};
}

#include <hyperapi/impl/OffsetTimestamp.impl.hpp>

#endif
