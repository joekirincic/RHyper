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

#ifndef TABLEAU_HYPER_TIME_HPP
#define TABLEAU_HYPER_TIME_HPP

#include <cstdint>
#include <hyperapi/SqlType.hpp>
#include <string>
#include <hyperapi/hyperapi.h>

namespace hyperapi {

/**
 * A time data value.
 */
class Time final {
   public:
   /**
     * Default constructor.
     */
   Time() noexcept {}

   /**
     * Creates a time value from separate time components.
     *
     * \param hour The hour.
     * \param minute The minute.
     * \param second The second.
     * \param microsecond The microsecond.
     * \pre `hour` must be between 0 and 23, `minute` and `second` between 0 and 59, and `microsecond` smaller than
     * 1'000'000.
     */
   Time(int8_t hour, std::int8_t minute, std::int8_t second, std::int32_t microsecond = 0) noexcept;

   /**
     * Gets the hour.
     *
     * \return The hour.
     */
   std::int8_t getHour() const noexcept;

   /**
     * Gets the minute.
     *
     * \return The minute.
     */
   std::int8_t getMinute() const noexcept;

   /**
     * Gets the second.
     *
     * \return The second.
     */
   std::int8_t getSecond() const noexcept;

   /**
     * Gets the microsecond.
     *
     * \return The microsecond.
     */
   std::int32_t getMicrosecond() const noexcept;

   /**
     * Gets a string representation for debugging.
     * \return the time in the 'HH:MM:SS.ssssss' format
     */
   std::string toString() const;
   /**
     * Equality operator.
     */
   friend bool operator==(const Time& lhs, const Time& rhs) noexcept;
   /**
     * Greater operator.
     */
   friend bool operator>(const Time& lhs, const Time& rhs) noexcept;
   /**
     * Not equal operator.
     */
   friend bool operator!=(const Time& a, const Time& b) noexcept { return !(a == b); }
   /**
     * Less than operator.
     */
   friend bool operator<(const Time& a, const Time& b) noexcept { return (b > a); }
   /**
     * Less than or equal operator.
     */
   friend bool operator<=(const Time& a, const Time& b) noexcept { return !(a > b); }
   /**
     * Greater or equal operator.
     */
   friend bool operator>=(const Time& a, const Time& b) noexcept { return !(a < b); }

   /** Stream output operator */
   friend std::ostream& operator<<(std::ostream& os, const Time& obj) { return os << obj.toString(); }

   private:
   friend class Inserter;
   friend class Value;
   friend class OffsetTimestamp;
   friend class Timestamp;
   friend struct std::hash<hyperapi::Time>;
   friend struct internal::ValueInserter;

   struct raw_t {};

   /**
     * Creates a time value from the number of microseconds since midnight.
     *
     * \param rawTime The raw time value.
     */
   explicit Time(hyper_time_t rawTime, raw_t) noexcept;

   /**
     * Gets the raw time value encoded as microseconds since midnight.
     *
     * \return The raw time value.
     */
   hyper_time_t getRaw() const noexcept;

   /**
     * The raw time value encoded as the number of microseconds since midnight.
     */
   hyper_time_t m_representation = 0;

   /** The parsed representation */
   hyper_time_components_t m_time = {0, 0, 0, 0};
};
}
#include <hyperapi/impl/Time.impl.hpp>

#endif
