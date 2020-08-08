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

#ifndef TABLEAU_HYPER_INTERVAL_HPP
#define TABLEAU_HYPER_INTERVAL_HPP

#include <cstdint>
#include <hyperapi/SqlType.hpp>
#include <string>
#include <hyperapi/hyperapi.h>

namespace hyperapi {

/**
 * An interval data value.
 */
class Interval final {
   public:
   /**
     * Default constructor.
     */
   Interval() noexcept {}

   /**
     * Creates an interval value from a number of years, months, days, hours, minutes, seconds, and microseconds.
     *
     * \param years The years.
     * \param months The months.
     * \param days The days.
     * \param hours The hours.
     * \param minutes The minutes.
     * \param seconds The seconds.
     * \param microseconds The microseconds.
     */
   Interval(
      std::int32_t years,
      std::int32_t months,
      std::int32_t days,
      std::int32_t hours,
      std::int32_t minutes,
      std::int32_t seconds,
      std::int32_t microseconds) noexcept;

   /**
     * Gets the number of years in the interval.
     *
     * \return The number of years.
     */
   std::int32_t getYears() const noexcept;

   /**
     * Gets the number of months in the interval.
     *
     * \return The number of months.
     */
   std::int32_t getMonths() const noexcept;

   /**
     * Gets the number of days in the interval.
     *
     * \return The number of days.
     */
   std::int32_t getDays() const noexcept;

   /**
     * Gets the number of hours in the interval.
     *
     * \return The number of hours.
     */
   std::int32_t getHours() const noexcept;

   /**
     * Gets the number of minutes in the interval.
     *
     * \return The number of minutes.
     */
   std::int32_t getMinutes() const noexcept;

   /**
     * Gets the number of seconds in the interval.
     *
     * \return The number of seconds.
     */
   std::int32_t getSeconds() const noexcept;

   /**
     * Gets the number of microseconds in the interval.
     *
     * \return The number of microseconds.
     */
   std::int32_t getMicroseconds() const noexcept;

   /**
     * Gets a string representation for debugging.
     * \return the interval in the 'P<date>T<time>' format while date is in the'[n]Y[n]M[n]D' format and time is in the
     * '[n]H[n]M[n]S' format. Elements that are 0 will be omitted.
     */
   std::string toString() const;
   /**
     * Equality operator.
     */
   friend bool operator==(const Interval& lhs, const Interval& rhs) noexcept;
   /**
     * Greater operator.
     */
   friend bool operator>(const Interval& lhs, const Interval& rhs) noexcept;
   /**
     * Not equal operator.
     */
   friend bool operator!=(const Interval& a, const Interval& b) noexcept { return !(a == b); }
   /**
     * Less than operator.
     */
   friend bool operator<(const Interval& a, const Interval& b) noexcept { return (b > a); }
   /**
     * Less than or equal operator.
     */
   friend bool operator<=(const Interval& a, const Interval& b) noexcept { return !(a > b); }
   /**
     * Greater or equal operator.
     */
   friend bool operator>=(const Interval& a, const Interval& b) noexcept { return !(a < b); }

   /** Stream output operator */
   friend std::ostream& operator<<(std::ostream& os, const Interval& obj) { return os << obj.toString(); }

   private:
   friend class Inserter;
   friend class Value;
   friend class Interval;
   friend struct std::hash<hyperapi::Interval>;
   friend struct internal::ValueInserter;

   struct raw_t {};

   /**
     * Creates an interval value from a raw interval value encoded as months, days, and microseconds.
     *
     * \param raw_interval The raw interval value.
     */
   explicit Interval(hyper_interval_t raw_interval, raw_t) noexcept;

   /**
     * Gets the raw interval value encoded as months, days, and microseconds.
     *
     * \return The raw interval value.
     */
   hyper_interval_t getRaw() const noexcept;

   /** Returns the internal month part. */
   int32_t getMonthPart() const noexcept;

   /** Returns the internal microseconds part. */
   int64_t getMicrosecondPart() const noexcept;

   private:
   /**
     * The raw interval value encoded as months (32 bits), days (32 bits), and microseconds (64 bits).
     */
   hyper_interval_t m_representation = {{0, 0}};

   /**
     * The interval components.
     */
   hyper_interval_components_t m_interval = {0, 0, 0, 0, 0, 0, 0};
};
}

#include <hyperapi/impl/Interval.impl.hpp>

#endif
