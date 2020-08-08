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

#ifndef TABLEAU_HYPER_IMPL_OFFSET_TIMESTAMP_IMPL_HPP
#define TABLEAU_HYPER_IMPL_OFFSET_TIMESTAMP_IMPL_HPP

#include <hyperapi/hyperapi.h>

namespace hyperapi {
inline OffsetTimestamp::OffsetTimestamp(Date date, Time time, std::chrono::minutes offset) noexcept
   : m_date(std::move(date)), m_time(std::move(time)), m_offset(std::move(offset)) {
   m_representation = this->m_date.getRaw() * microseconds_per_day + this->m_time.getRaw();
   int64_t offsetUs = std::chrono::microseconds(m_offset).count();
   assert(((offsetUs < 0) || (static_cast<hyper_timestamp_t>(offsetUs) < m_representation)) && "offset timestamp before minimum UTC time point");
   if (offsetUs >= 0) {
      m_representation -= static_cast<hyper_timestamp_t>(offsetUs);
   } else {
      m_representation += static_cast<hyper_timestamp_t>(-offsetUs);
   }
}

inline OffsetTimestamp::OffsetTimestamp(hyper_timestamp_t rawTimestamp, raw_t) noexcept
   : m_representation(rawTimestamp) {
   m_date = hyperapi::Date(static_cast<hyper_date_t>(m_representation / microseconds_per_day), Date::raw_t());
   m_time = hyperapi::Time(static_cast<hyper_time_t>(m_representation % microseconds_per_day), Time::raw_t());
}

inline hyper_timestamp_t OffsetTimestamp::getRaw() const noexcept {
   return m_representation;
}

inline const Date& OffsetTimestamp::getDate() const noexcept {
   return m_date;
}

inline const Time& OffsetTimestamp::getTime() const noexcept {
   return m_time;
}

inline std::chrono::minutes OffsetTimestamp::getOffset() const noexcept {
   return m_offset;
}

inline std::string OffsetTimestamp::toString() const {
   std::ostringstream stream;
   stream << m_date << ' ' << m_time;
   stream << ((m_offset.count() < 0) ? '-' : '+');
   stream << std::setw(2) << std::setfill('0') << (abs(m_offset.count()) / 60);
   stream << ":";
   stream << std::setw(2) << std::setfill('0') << (abs(m_offset.count()) % 60);
   return stream.str();
}

inline bool operator==(const OffsetTimestamp& lhs, const OffsetTimestamp& rhs) noexcept {
   return lhs.m_representation == rhs.m_representation;
}

inline bool operator>(const OffsetTimestamp& lhs, const OffsetTimestamp& rhs) noexcept {
   return lhs.m_representation > rhs.m_representation;
}
}

namespace std {
/** Specialization of `std::hash` for `hyperapi::OffsetTimestamp`. */
template <>
struct hash<hyperapi::OffsetTimestamp> {
   /** Calculates the hash value of the given timestamp. */
   size_t operator()(const hyperapi::OffsetTimestamp& timestamp) const noexcept { return timestamp.getRaw(); }
};
}
#endif
