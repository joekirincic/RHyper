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

#ifndef TABLEAU_HYPER_IMPL_SQLTYPE_IMPL_HPP
#define TABLEAU_HYPER_IMPL_SQLTYPE_IMPL_HPP

#include <hyperapi/string_view.hpp>
#include <iostream>
#include <sstream>
#include <vector>

namespace hyperapi {

template <unsigned precision, unsigned scale>
class Numeric;
struct ByteSpan;
class Interval;
class Time;
class Date;
class Timestamp;
class OffsetTimestamp;

namespace internal {
inline std::string getNativeTypeName(SqlType type) noexcept {
   switch (type.getTag()) {
      case TypeTag::Bool:
         return "bool";
      case TypeTag::BigInt:
         return "int64_t";
      case TypeTag::SmallInt:
         return "int16_t";
      case TypeTag::Int:
         return "int32_t";
      case TypeTag::Numeric:
         return "Numeric<" + std::to_string(type.getPrecision()) + "," + std::to_string(type.getScale()) + ">";
      case TypeTag::Double:
         return "double";
      case TypeTag::Oid:
         return "uint32_t";
      case TypeTag::Bytes:
      case TypeTag::Geography:
      case TypeTag::Unsupported:
         return "ByteSpan";
      case TypeTag::Text:
      case TypeTag::Varchar:
      case TypeTag::Char:
      case TypeTag::Json:
         return "string";
      case TypeTag::Date:
         return "Date";
      case TypeTag::Time:
         return "Time";
      case TypeTag::Interval:
         return "Interval";
      case TypeTag::Timestamp:
         return "Timestamp";
      case TypeTag::TimestampTZ:
         return "OffsetTimestamp";
   }

   std::abort(); // unreachable
}

template <typename T>
struct false_type {
   static constexpr bool value = false;
};

template <class T>
struct SqlTypeGetter {
   SqlType get() noexcept { static_assert(false_type<T>::value, "This data type has no corresponding SqlType."); }
};
template <class T>
struct SqlTypeGetter<optional<T>> {
   inline SqlType get() noexcept { return SqlTypeGetter<T>().get(); }
};
template <unsigned precision, unsigned scale>
struct SqlTypeGetter<Numeric<precision, scale>> {
   inline SqlType get() noexcept { return SqlType::numeric(precision, scale); }
};
template <class T>
inline SqlType getIntegralType() noexcept {
   static_assert((sizeof(T) == 2) || (sizeof(T) == 4) || (sizeof(T) == 8), "Unexpected `sizeof(T)`");
   switch (sizeof(T)) {
      case 2: return SqlType::smallInt();
      case 4: return SqlType::integer();
      case 8: return SqlType::bigInt();
   }
   HYPER_UNREACHABLE();
}
template <>
struct SqlTypeGetter<short> {
   inline SqlType get() noexcept { return getIntegralType<short>(); }
};
template <>
struct SqlTypeGetter<int> {
   inline SqlType get() noexcept { return getIntegralType<int>(); }
};
template <>
struct SqlTypeGetter<long> {
   inline SqlType get() noexcept { return getIntegralType<long>(); }
};
template <>
struct SqlTypeGetter<long long> {
   inline SqlType get() noexcept { return getIntegralType<long long>(); }
};
template <>
struct SqlTypeGetter<double> {
   inline SqlType get() noexcept { return SqlType::doublePrecision(); }
};
template <>
struct SqlTypeGetter<long double> {
   inline SqlType get() noexcept { return SqlType::doublePrecision(); }
};
template <>
struct SqlTypeGetter<bool> {
   inline SqlType get() noexcept { return SqlType::boolean(); }
};
template <>
struct SqlTypeGetter<uint32_t> {
   inline SqlType get() noexcept { return SqlType::oid(); }
};
template <>
struct SqlTypeGetter<ByteSpan> {
   inline SqlType get() noexcept { return SqlType::bytes(); }
};
template <>
struct SqlTypeGetter<std::vector<uint8_t>> {
   inline SqlType get() noexcept { return SqlType::bytes(); }
};
template <>
struct SqlTypeGetter<string_view> {
   inline SqlType get() noexcept { return SqlType::text(); }
};
template <>
struct SqlTypeGetter<const char*> {
   inline SqlType get() noexcept { return SqlType::text(); }
};
template <>
struct SqlTypeGetter<std::string> {
   inline SqlType get() noexcept { return SqlType::text(); }
};
template <>
struct SqlTypeGetter<Interval> {
   inline SqlType get() noexcept { return SqlType::interval(); }
};
template <>
struct SqlTypeGetter<Date> {
   inline SqlType get() noexcept { return SqlType::date(); }
};
template <>
struct SqlTypeGetter<Time> {
   inline SqlType get() noexcept { return SqlType::time(); }
};
template <>
struct SqlTypeGetter<Timestamp> {
   inline SqlType get() noexcept { return SqlType::timestamp(); }
};
template <>
struct SqlTypeGetter<OffsetTimestamp> {
   inline SqlType get() noexcept { return SqlType::timestampTZ(); }
};
template <class T>
inline SqlType getSqlType() noexcept {
   return SqlTypeGetter<T>().get();
}
}

inline uint32_t SqlType::getPrecision() const noexcept {
   if (tag == TypeTag::Numeric) {
      return hyper_get_precision_from_modifier(modifier);
   } else {
      return HYPER_UNUSED_MODIFIER;
   }
}

inline uint32_t SqlType::getScale() const noexcept {
   if (tag == TypeTag::Numeric) {
      return hyper_get_scale_from_modifier(modifier);
   } else {
      return HYPER_UNUSED_MODIFIER;
   }
}

inline uint32_t SqlType::getMaxLength() const noexcept {
   if ((tag == TypeTag::Char) || (tag == TypeTag::Varchar)) {
      return hyper_get_max_length_from_modifier(modifier);
   } else {
      return HYPER_UNUSED_MODIFIER;
   }
}

inline std::ostream& operator<<(std::ostream& os, const SqlType& type) {
   switch (type.getTag()) {
      case TypeTag::Unsupported:
         return os << "UNSUPPORTED";
      case TypeTag::Bool:
         return os << "BOOLEAN";
      case TypeTag::BigInt:
         return os << "BIGINT";
      case TypeTag::SmallInt:
         return os << "SMALLINT";
      case TypeTag::Int:
         return os << "INTEGER";
      case TypeTag::Double:
         return os << "DOUBLE PRECISION";
      case TypeTag::Numeric:
         return os << "NUMERIC(" << type.getPrecision() << ',' << type.getScale() << ')';
      case TypeTag::Oid:
         return os << "OID";
      case TypeTag::Bytes:
         return os << "BYTEA";
      case TypeTag::Text:
         return os << "TEXT";
      case TypeTag::Varchar:
         return os << "VARCHAR(" << type.getMaxLength() << ")";
      case TypeTag::Char:
         return os << "CHARACTER(" << type.getMaxLength() << ")";
      case TypeTag::Json:
         return os << "JSON";
      case TypeTag::Date:
         return os << "DATE";
      case TypeTag::Interval:
         return os << "INTERVAL";
      case TypeTag::Time:
         return os << "TIME";
      case TypeTag::Timestamp:
         return os << "TIMESTAMP";
      case TypeTag::TimestampTZ:
         return os << "TIMESTAMPTZ";
      case TypeTag::Geography:
         return os << "GEOGRAPHY";
   }
   std::abort(); // unreachable
}

inline std::string SqlType::toString() const {
   std::ostringstream ss;
   ss << *this;
   return ss.str();
}

inline bool operator==(const SqlType& a, const SqlType& b) noexcept {
   return (a.tag == b.tag) && (a.internalOid == b.internalOid) && (a.modifier == b.modifier);
}

inline bool operator>(const SqlType& a, const SqlType& b) noexcept {
   if (a.tag != b.tag) {
      return a.tag > b.tag;
   } else if (a.internalOid != b.internalOid) {
      return a.internalOid > b.internalOid;
   } else {
      return a.modifier > b.modifier;
   }
}

inline bool operator!=(const SqlType& a, const SqlType& b) noexcept {
   return !(a == b);
}

inline bool operator<(const SqlType& a, const SqlType& b) noexcept {
   return (b > a);
}

inline bool operator<=(const SqlType& a, const SqlType& b) noexcept {
   return !(a > b);
}

inline bool operator>=(const SqlType& a, const SqlType& b) noexcept {
   return !(a < b);
}
}

#endif
