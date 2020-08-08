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

#ifndef TABLEAU_HYPER_IMPL_TIMESTAMP_IMPL_HPP
#define TABLEAU_HYPER_IMPL_TIMESTAMP_IMPL_HPP

#include <hyperapi/hyperapi.h>

namespace hyperapi {
inline Timestamp::Timestamp(Date date, Time time) noexcept
   : m_date(std::move(date)), m_time(std::move(time)) {
   m_representation = this->m_date.getRaw() * microseconds_per_day + this->m_time.getRaw();
}

inline Timestamp::Timestamp(hyper_timestamp_t rawTimestamp, raw_t) noexcept
   : m_representation(rawTimestamp) {
   m_date = hyperapi::Date(static_cast<hyper_date_t>(m_representation / microseconds_per_day), Date::raw_t());
   m_time = hyperapi::Time(static_cast<hyper_time_t>(m_representation % microseconds_per_day), Time::raw_t());
}

inline hyper_timestamp_t Timestamp::getRaw() const noexcept {
   return m_representation;
}

inline const Date& Timestamp::getDate() const noexcept {
   return m_date;
}

inline const Time& Timestamp::getTime() const noexcept {
   return m_time;
}

inline std::string Timestamp::toString() const {
   return getDate().toString() + ' ' + getTime().toString();
}

inline bool operator==(const Timestamp& lhs, const Timestamp& rhs) noexcept {
   return lhs.m_representation == rhs.m_representation;
}

inline bool operator>(const Timestamp& lhs, const Timestamp& rhs) noexcept {
   return lhs.m_representation > rhs.m_representation;
}
}

namespace std {
/** Specialization of `std::hash` for `hyperapi::Timestamp`. */
template <>
struct hash<hyperapi::Timestamp> {
   /** Calculates the hash value of the given timestamp. */
   size_t operator()(const hyperapi::Timestamp& timestamp) const noexcept { return timestamp.getRaw(); }
};
}
#endif
