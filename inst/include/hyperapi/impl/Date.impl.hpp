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

#ifndef TABLEAU_HYPER_IMPL_DATE_IMPL_HPP
#define TABLEAU_HYPER_IMPL_DATE_IMPL_HPP

#include <iomanip>
#include <hyperapi/hyperapi.h>

namespace hyperapi {
inline Date::Date(int32_t year, int16_t month, int16_t day) noexcept
   : Date(hyper_encode_date({year, month, day}), raw_t()) {
   assert(((year > -4800)) && "Only years after 4800 BC supported");
   assert(((month >= 1) && (month <= 12)) && "A valid month must be between 1 and 12");
   assert(((day >= 1) && (day <= 31)) && "A valid day must be between 1 and 31");
}

inline Date::Date(hyper_date_t rawDate, raw_t) noexcept
   : m_representation(rawDate) {
   hyper_date_components_t result = hyper_decode_date(m_representation);
   m_date.year = result.year;
   m_date.month = result.month;
   m_date.day = result.day;
}

inline hyper_date_t Date::getRaw() const noexcept {
   return m_representation;
}

inline std::int16_t Date::getDay() const noexcept {
   return m_date.day;
}

inline std::int16_t Date::getMonth() const noexcept {
   return m_date.month;
}

inline std::int32_t Date::getYear() const noexcept {
   return m_date.year;
}

inline std::string Date::toString() const {
   std::ostringstream ss;
   ss << std::setfill('0');
   if (getYear() < 0) {
      ss << "-" << std::setw(4) << std::to_string(-getYear());
   } else {
      ss << std::setw(4) << std::to_string(getYear());
   }
   ss << "-" << std::setw(2) << std::to_string(getMonth());
   ss << "-" << std::setw(2) << std::to_string(getDay());
   return ss.str();
}

inline bool operator==(const Date& lhs, const Date& rhs) noexcept {
   return lhs.m_representation == rhs.m_representation;
}

inline bool operator>(const Date& lhs, const Date& rhs) noexcept {
   return lhs.m_representation > rhs.m_representation;
}
}

namespace std {
/** Specialization of `std::hash` for `hyperapi::Date`. */
template <>
struct hash<hyperapi::Date> {
   /** Calculates the hash value of the given date. */
   size_t operator()(const hyperapi::Date& date) const noexcept { return date.getRaw(); }
};
}
#endif
