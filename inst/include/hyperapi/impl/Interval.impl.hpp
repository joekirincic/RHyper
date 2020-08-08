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

#ifndef TABLEAU_HYPER_IMPL_INTERVAL_IMPL_HPP
#define TABLEAU_HYPER_IMPL_INTERVAL_IMPL_HPP

#include <hyperapi/hyperapi.h>

#include <cassert>
#include <iomanip>
#include <sstream>
#include <utility>

namespace hyperapi {

inline Interval::Interval(int32_t years, int32_t months, int32_t days, int32_t hours, int32_t minutes, int32_t seconds, int32_t microseconds) noexcept
   : Interval(hyper_encode_interval({years, months, days, hours, minutes, seconds, microseconds}), raw_t()) {
}

inline Interval::Interval(hyper_interval_t raw_interval, raw_t) noexcept
   : m_representation(raw_interval) {
   hyper_interval_components_t result = hyper_decode_interval(m_representation);
   m_interval.years = result.years;
   m_interval.months = result.months;
   m_interval.days = result.days;
   m_interval.hours = result.hours;
   m_interval.minutes = result.minutes;
   m_interval.seconds = result.seconds;
   m_interval.microseconds = result.microseconds;
}

inline hyper_interval_t Interval::getRaw() const noexcept {
   return m_representation;
}

inline int32_t Interval::getYears() const noexcept {
   return m_interval.years;
}

inline int32_t Interval::getMonths() const noexcept {
   return m_interval.months;
}

inline int32_t Interval::getDays() const noexcept {
   return m_interval.days;
}

inline int32_t Interval::getHours() const noexcept {
   return m_interval.hours;
}

inline int32_t Interval::getMinutes() const noexcept {
   return m_interval.minutes;
}

inline int32_t Interval::getSeconds() const noexcept {
   return m_interval.seconds;
}

inline int32_t Interval::getMicroseconds() const noexcept {
   return m_interval.microseconds;
}

inline std::string Interval::toString() const {
   auto addIfNotZero = [](int32_t value, std::ostringstream& ss, char c) {
      if (value != 0) {
         ss << std::to_string(value) << c;
      }
   };

   bool printDate = (getYears() != 0) || (getMonths() != 0) || (getDays() != 0);
   bool printTime = (getHours() != 0) || (getMinutes() != 0) || (getSeconds() != 0) || (getMicroseconds() != 0);

   std::ostringstream ss;

   ss << "P";
   addIfNotZero(getYears(), ss, 'Y');
   addIfNotZero(getMonths(), ss, 'M');
   addIfNotZero(getDays(), ss, 'D');

   // Time part only printed if it is not zero
   if (printTime) {
      ss << "T";
      addIfNotZero(getHours(), ss, 'H');
      addIfNotZero(getMinutes(), ss, 'M');

      // Seconds and microseconds are merged together
      if ((getSeconds() != 0) || (getMicroseconds() != 0)) {
         static const int32_t e6 = 1000000;
         int32_t microSecondsSum = getSeconds() * e6 + getMicroseconds();
         ss << ((microSecondsSum >= 0) ? "" : "-");
         int32_t microSecondsAbsolute = ((microSecondsSum < 0) ? -microSecondsSum : microSecondsSum);
         ss << std::to_string(microSecondsAbsolute / e6);
         int32_t microSecondsRemainder = microSecondsAbsolute % e6;
         if (microSecondsRemainder != 0) {
            ss << "." << std::setfill('0') << std::setw(6) << microSecondsRemainder;
         }
         ss << "S";
      }
   }

   if (!printDate && !printTime) {
      // We only printed P up to this point which is not a valid interval string on its own.
      ss << "T0S";
   }

   return ss.str();
}

inline int32_t Interval::getMonthPart() const noexcept {
   return (getYears() * 12 + getMonths());
}

inline int64_t Interval::getMicrosecondPart() const noexcept {
   union {
      uint64_t us;
      int64_t s;
   } cast;
   cast.us = m_representation.data[0];
   return cast.s;
}

inline bool operator==(const Interval& lhs, const Interval& rhs) noexcept {
   // To match Hyper's comparison, we have to compare three individual parts: months, days and microseconds.
   return (lhs.getMonthPart() == rhs.getMonthPart()) && (lhs.getDays() == rhs.getDays()) && (lhs.getMicrosecondPart() == rhs.getMicrosecondPart());
}

inline bool operator>(const Interval& a, const Interval& b) noexcept {
   // To match Hyper's comparison, we have to compare three individual parts: months, days and microseconds.
   int32_t aMonths = a.getMonthPart();
   int32_t bMonths = b.getMonthPart();
   int64_t aMicroSeconds = a.getMicrosecondPart();
   int64_t bMicroSeconds = b.getMicrosecondPart();

   if (aMonths != bMonths) {
      return aMonths > bMonths;
   }
   if (a.getDays() != b.getDays()) {
      return a.getDays() > b.getDays();
   }
   return aMicroSeconds > bMicroSeconds;
}
}

namespace std {
/** Specialization of `std::hash` for `hyperapi::Interval`. */
template <>
struct hash<hyperapi::Interval> {
   /** Calculates the hash value of the given interval. */
   size_t operator()(const hyperapi::Interval& interval) const noexcept { return interval.getRaw().data[0] ^ interval.getRaw().data[1]; }
};
}

#endif
