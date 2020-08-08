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

#ifndef TABLEAU_HYPER_IMPL_TIME_IMPL_HPP
#define TABLEAU_HYPER_IMPL_TIME_IMPL_HPP

#include <hyperapi/hyperapi.h>

#include <cassert>
#include <iomanip>
#include <sstream>
#include <utility>

namespace hyperapi {

inline Time::Time(int8_t hour, int8_t minute, int8_t second, int32_t microsecond) noexcept
   : Time(hyper_encode_time({hour, minute, second, microsecond}), raw_t()) {
   assert(hour < 24);
   assert(minute < 60);
   assert(second < 60);
   assert(microsecond < 1000000);
}

inline Time::Time(hyper_time_t rawTime, raw_t) noexcept
   : m_representation(rawTime) {
   hyper_time_components_t result = hyper_decode_time(m_representation);
   m_time.hour = result.hour;
   m_time.minute = result.minute;
   m_time.second = result.second;
   m_time.microsecond = result.microsecond;
}

inline hyper_time_t Time::getRaw() const noexcept {
   return m_representation;
}

inline int8_t Time::getHour() const noexcept {
   return m_time.hour;
}

inline int8_t Time::getMinute() const noexcept {
   return m_time.minute;
}

inline int8_t Time::getSecond() const noexcept {
   return m_time.second;
}

inline int32_t Time::getMicrosecond() const noexcept {
   return m_time.microsecond;
}

inline std::string Time::toString() const {
   std::ostringstream ss;
   ss << std::setfill('0');
   ss << std::setw(2) << std::to_string(getHour());
   ss << ":" << std::setw(2) << std::to_string(getMinute());
   ss << ":" << std::setw(2) << std::to_string(getSecond());
   if (getMicrosecond()) {
      ss << "." << std::setw(6) << std::to_string(getMicrosecond());
   }
   return ss.str();
}

inline bool operator==(const Time& lhs, const Time& rhs) noexcept {
   return lhs.m_representation == rhs.m_representation;
}

inline bool operator>(const Time& lhs, const Time& rhs) noexcept {
   return lhs.m_representation > rhs.m_representation;
}
}

namespace std {
/** Specialization of `std::hash` for `hyperapi::Time`. */
template <>
struct hash<hyperapi::Time> {
   /** Calculates the hash value of the given time. */
   size_t operator()(const hyperapi::Time& time) const noexcept { return time.getRaw(); }
};
}

#endif
