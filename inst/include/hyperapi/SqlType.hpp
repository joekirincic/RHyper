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

#ifndef TABLEAU_HYPER_SQLTYPE_HPP
#define TABLEAU_HYPER_SQLTYPE_HPP

#include <hyperapi/impl/infra.hpp>
#include <string>
#include <hyperapi/hyperapi.h>

namespace hyperapi {

/** A generic null value for insertion */
static CONSTEXPR_OPTIONAL optional<internal::AnyType> null = {};

/** A type tag. */
enum class TypeTag : int {
   Unsupported = HYPER_UNSUPPORTED,
   Bool = HYPER_BOOL,
   BigInt = HYPER_BIG_INT,
   SmallInt = HYPER_SMALL_INT,
   Int = HYPER_INT,
   Numeric = HYPER_NUMERIC,
   Double = HYPER_DOUBLE,
   Oid = HYPER_OID,
   Bytes = HYPER_BYTE_A,
   Text = HYPER_TEXT,
   Varchar = HYPER_VARCHAR,
   Char = HYPER_CHAR,
   Json = HYPER_JSON,
   Date = HYPER_DATE,
   Interval = HYPER_INTERVAL,
   Time = HYPER_TIME,
   Timestamp = HYPER_TIMESTAMP,
   TimestampTZ = HYPER_TIMESTAMP_TZ,
   Geography = HYPER_GEOGRAPHY
};

/**
 * A Hyper SQL type.
 */
class SqlType final {
   public:
   /// Constructor
   SqlType(TypeTag tag, uint32_t oid, hyper_type_modifier_t modifier = HYPER_UNUSED_MODIFIER) noexcept
      : tag(tag), internalOid(oid), modifier(modifier) {
   }

   /**
     * Returns the type tag.
     */
   TypeTag getTag() const noexcept { return tag; }

   /**
     * Returns the internal oid.
     *
     * Note: This method is internal and may therefore be subject to change in future versions of the API.
     */
   uint32_t getInternalOid() const noexcept { return internalOid; }

   /**
     * Returns the internal type modifier.
     *
     * Note: This method is internal and may therefore be subject to change in future versions of the API.
     */
   uint32_t getInternalTypeModifier() const noexcept { return modifier; }

   /**
     * Returns the maximum precision parameter of the type if the type supports it, HYPER_UNUSED_MODIFIER otherwise.
     */
   uint32_t getPrecision() const noexcept;

   /**
     * Returns the scale parameter of the type if the type supports it, HYPER_UNUSED_MODIFIER otherwise.
     */
   uint32_t getScale() const noexcept;

   /**
     * Returns the maximum length parameter of the type if the type supports it, HYPER_UNUSED_MODIFIER otherwise.
     */
   uint32_t getMaxLength() const noexcept;

   /**
     * Returns a string representation for debugging.
     *
     * The returned string will be the SQL name of the type.
     */
   std::string toString() const;

   /**
     * Returns the BOOL SQL type.
     * \return The BOOL SQL type
     */
   static SqlType boolean() noexcept { return {TypeTag::Bool, HYPER_OID_BOOL}; }
   /**
     * Returns the BIG INTEGER SQL type.
     * \return The BIG INTEGER SQL type
     */
   static SqlType bigInt() noexcept { return {TypeTag::BigInt, HYPER_OID_BIG_INT}; }
   /**
     * Returns the SMALL INTEGER SQL type.
     * \return The SMALL INTEGER SQL type
     */
   static SqlType smallInt() noexcept { return {TypeTag::SmallInt, HYPER_OID_SMALL_INT}; }
   /**
     * Returns the INTEGER SQL type.
     * \return The INTEGER SQL type
     */
   static SqlType integer() noexcept { return {TypeTag::Int, HYPER_OID_INT}; }
   /**
     * Returns the NUMERIC SQL type.
     * \param precision  The precision
     * \param scale  The scale
     * \return The NUMERIC SQL type
     */
   static SqlType numeric(uint16_t precision, uint16_t scale) noexcept { return {TypeTag::Numeric, HYPER_OID_NUMERIC, hyper_encode_numeric_modifier(precision, scale)}; }
   /**
     * Returns the DOUBLE PRECISION SQL type.
     * \return The DOUBLE PRECISION SQL type
     */
   static SqlType doublePrecision() noexcept { return {TypeTag::Double, HYPER_OID_DOUBLE}; }
   /**
     * Returns the OID SQL type.
     * \return The OID SQL type
     */
   static SqlType oid() noexcept { return {TypeTag::Oid, HYPER_OID_OID}; }
   /**
     * Returns the BYTEA SQL type.
     * \return The BYTEA SQL type
     */
   static SqlType bytes() noexcept { return {TypeTag::Bytes, HYPER_OID_BYTE_A}; }
   /**
     * Returns the TEXT SQL type.
     * \return The TEXT SQL type
     */
   static SqlType text() noexcept { return {TypeTag::Text, HYPER_OID_TEXT}; }
   /**
     * Returns the VARCHAR SQL type.
     * \param maxLength  The maximum length
     * \return The VARCHAR SQL type
     */
   static SqlType varchar(uint32_t maxLength) noexcept { return {TypeTag::Varchar, HYPER_OID_VARCHAR, hyper_encode_string_modifier(maxLength)}; }
   /**
     * Returns the CHARACTER SQL type.
     * \param maxLength  The maximum length
     * \return The CHARACTER SQL type
     */
   static SqlType character(uint32_t maxLength) noexcept { return {TypeTag::Char, static_cast<uint32_t>((maxLength == 1) ? HYPER_OID_CHAR1 : HYPER_OID_CHAR), hyper_encode_string_modifier(maxLength)}; }
   /**
     * Returns the JSON SQL type.
     * \return The JSON SQL type
     */
   static SqlType json() noexcept { return {TypeTag::Json, HYPER_OID_JSON}; }
   /**
     * Returns the DATE SQL type.
     * \return The DATE SQL type
     */
   static SqlType date() noexcept { return {TypeTag::Date, HYPER_OID_DATE}; }
   /**
     * Returns the INTERVAL SQL type.
     * \return The INTERVAL SQL type
     */
   static SqlType interval() noexcept { return {TypeTag::Interval, HYPER_OID_INTERVAL}; }
   /**
     * Returns the TIME SQL type.
     * \return The TIME SQL type
     */
   static SqlType time() noexcept { return {TypeTag::Time, HYPER_OID_TIME}; }
   /**
     * Returns the TIMESTAMP SQL type.
     * \return The TIMESTAMP SQL type
     */
   static SqlType timestamp() noexcept { return {TypeTag::Timestamp, HYPER_OID_TIMESTAMP}; }
   /**
     * Returns the TIMESTAMPTZ SQL type.
     * \return The TIMESTAMPTZ SQL type
     */
   static SqlType timestampTZ() noexcept { return {TypeTag::TimestampTZ, HYPER_OID_TIMESTAMP_TZ}; }
   /**
     * Returns the GEOGRAPHY SQL type.
     * \return The GEOGRAPHY SQL type
     */
   static SqlType geography() noexcept { return {TypeTag::Geography, HYPER_OID_GEOGRAPHY}; }

   /**
     * Equality operator.
     */
   friend bool operator==(const SqlType& a, const SqlType& b) noexcept;
   /**
     * Greater operator.
     */
   friend bool operator>(const SqlType& a, const SqlType& b) noexcept;
   /**
     * Not equal operator.
     */
   friend bool operator!=(const SqlType& a, const SqlType& b) noexcept;
   /**
     * Smaller operator.
     */
   friend bool operator<(const SqlType& a, const SqlType& b) noexcept;
   /**
     * Smaller or equal operator.
     */
   friend bool operator<=(const SqlType& a, const SqlType& b) noexcept;
   /**
     * Greater or equal operator.
     */
   friend bool operator>=(const SqlType& a, const SqlType& b) noexcept;
   /**
     * Stream output operator
     */
   friend std::ostream& operator<<(std::ostream& os, const SqlType& obj);

   private:
   /// The type tag
   TypeTag tag;
   /// The OID
   uint32_t internalOid;
   /// The type modifier
   hyper_type_modifier_t modifier;

   friend class internal::HyperTableDefinition;
   friend struct std::hash<hyperapi::SqlType>;
};
}

namespace std {

/** Specialization of `std::hash` for `hyperapi::SqlType`. */
template <>
struct hash<hyperapi::SqlType> {
   /** Calculates the hash value of the given SQL type. */
   size_t operator()(const hyperapi::SqlType& type) const noexcept { return (static_cast<size_t>(type.tag) << 32) | type.modifier; }
};
}

#include <hyperapi/impl/SqlType.impl.hpp>

#endif
