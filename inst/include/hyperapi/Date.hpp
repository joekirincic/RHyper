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

#ifndef TABLEAU_HYPER_DATE_HPP
#define TABLEAU_HYPER_DATE_HPP

#include <cstdint>
#include <hyperapi/SqlType.hpp>
#include <string>
#include <hyperapi/hyperapi.h>

namespace hyperapi {

/**
 * A date data value.
 */
class Date final {
   public:
   /**
     * Default constructor.
     */
   Date() noexcept {}

   /**
     * Creates a date value.
     *
     * \param year The year. To specify BC dates, you must convert the BC year to a negative year (e.g., 10 BC = -9).
     * \param month The month.
     * \param day The day.
     * \pre `year`, `month`, and `day` must define a valid date after 4800 BC.
     */
   Date(int32_t year, std::int16_t month, std::int16_t day) noexcept;

   /**
     * Gets the day.
     *
     * \return The day.
     */
   std::int16_t getDay() const noexcept;

   /**
     * Gets the month.
     *
     * \return The month.
     */
   std::int16_t getMonth() const noexcept;

   /**
     * Gets the year.
     *
     * \return The year.
     */
   std::int32_t getYear() const noexcept;

   /**
     * Gets a string representation for debugging.
     * \return the date in the '±YYYY-MM-DD' format
     */
   std::string toString() const;
   /**
     * Equality operator.
     */
   friend bool operator==(const Date& a, const Date& b) noexcept;
   /**
     * Greater operator.
     */
   friend bool operator>(const Date& a, const Date& b) noexcept;
   /**
     * Not equal operator.
     */
   friend bool operator!=(const Date& a, const Date& b) noexcept { return !(a == b); }
   /**
     * Less than operator.
     */
   friend bool operator<(const Date& a, const Date& b) noexcept { return (b > a); }
   /**
     * Less than or equal operator.
     */
   friend bool operator<=(const Date& a, const Date& b) noexcept { return !(a > b); }
   /**
     * Greater or equal operator.
     */
   friend bool operator>=(const Date& a, const Date& b) noexcept { return !(a < b); }

   /** Stream output operator */
   friend std::ostream& operator<<(std::ostream& os, const Date& obj) { return os << obj.toString(); }

   private:
   friend class Inserter;
   friend class OffsetTimestamp;
   friend class Timestamp;
   friend struct std::hash<hyperapi::Date>;
   friend class Value;
   friend struct internal::ValueInserter;

   struct raw_t {};

   /**
     * Creates a date value from a raw date value (i.e., a Julian Day).
     *
     * \param rawDate The raw date value.
     */
   explicit Date(hyper_date_t rawDate, raw_t) noexcept;

   /**
     * Gets the raw date value (i.e., the Julian Day).
     *
     * \return The raw date value.
     */
   hyper_date_t getRaw() const noexcept;

   /**
     * The raw date value. This is the Julian Day, which is the number of days since 1 January 4713 BC.
     */
   hyper_date_t m_representation = 0;

   /** The parsed representation */
   hyper_date_components_t m_date = {0, 0, 0};
};
}

#include <hyperapi/impl/Date.impl.hpp>

#endif
