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

#ifndef TABLEAU_HYPER_IMPL_RESULT_IMPL_HPP
#define TABLEAU_HYPER_IMPL_RESULT_IMPL_HPP

#include <cassert>
#include <cctype>
#include <hyperapi/Connection.hpp>
#include <sstream>
#include <string>
#include <hyperapi/hyperapi.h>

namespace hyperapi {

namespace internal {
[[noreturn]] inline void throwTypeError(string_view requestedType, SqlType actualType, string_view columnName) {
   std::string message = "Type conflict on Value access for column '" + std::string(columnName) + "'. " + "Requested type: '" + std::string(requestedType) +
      "' but actually got: '" + actualType.toString() + "'. Request: '" + getNativeTypeName(actualType) + "'.";
   HYPER_PRECONDITION_MSG(false, message);
   std::abort(); // unreachable
}

inline std::string createNullabilityErrorMessage(string_view requestedType, string_view columnName) {
   return "Type conflict on Value access for column '" + std::string(columnName) + "'. " + "Requested type: '" + std::string(requestedType) +
      "' but actually got: NULL. Try requesting 'optional<" + std::string(requestedType) + ">'.";
}

template <class T>
inline T getIntegral(string_view requestedType, SqlType type, hyper_value_t value, string_view columnName) {
   switch (type.getTag()) {
      case TypeTag::SmallInt: {
         assert(value.size == sizeof(int16_t));
         int16_t v = hyper_read_int16(value.value);
         if (sizeof(T) < 2) {
            break;
         }
         return static_cast<T>(v);
      }
      case TypeTag::Int: {
         assert(value.size == sizeof(int32_t));
         int32_t v = hyper_read_int32(value.value);
         if (sizeof(T) < 4) {
            break;
         }
         return static_cast<T>(v);
      }
      case TypeTag::BigInt: {
         assert(value.size == sizeof(int64_t));
         int64_t v = hyper_read_int64(value.value);
         if (sizeof(T) < 8) {
            break;
         }
         return static_cast<T>(v);
      }
      default:
         break;
   }
   throwTypeError(requestedType, type, columnName);
}

template <class ReturnType>
struct ValueAccess {
   ReturnType getValue(Value) { static_assert(false_type<ReturnType>::value, "This data type cannot be used to retrieve a result value."); }
};

template <unsigned precision, unsigned scale>
struct ValueAccess<Numeric<precision, scale>> {
   inline Numeric<precision, scale> getValue(Value value) {
      HYPER_PRECONDITION_MSG(value.value.value, internal::createNullabilityErrorMessage("Numeric<" + std::to_string(precision) + "," + std::to_string(scale) + ">", value.columnName));
      switch (value.type.getTag()) {
         case TypeTag::SmallInt:
            return value.get<int16_t>();
         case TypeTag::Int:
            return value.get<int32_t>();
         case TypeTag::BigInt:
            return value.get<int64_t>();
         case TypeTag::Double:
            return value.get<double>();
         case TypeTag::Numeric:
            assert(value.value.size == sizeof(int64_t));
            return Numeric<precision, scale>(hyper_read_int64(value.value.value), value.type.getPrecision(), value.type.getScale(), typename Numeric<precision, scale>::raw_t());
         default:
            internal::throwTypeError("Numeric<" + std::to_string(precision) + "," + std::to_string(scale) + ">", value.type, value.columnName);
      }
   }
};

template <class ReturnType>
struct ValueAccess<optional<ReturnType>> {
   inline optional<ReturnType> getValue(Value value) {
      if (value.isNull()) {
         return {};
      }
      return value.get<ReturnType>();
   }
};

template <typename T, typename F>
T bitCast(F f) noexcept {
   static_assert(sizeof(T) == sizeof(F), "size mismatch");
   union {
      F f;
      T t;
   } u{f};
   return u.t;
}
}

template <>
inline short Value::get<short>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("short", columnName));
   return internal::getIntegral<short>("short", type, value, columnName);
}

template <>
inline int Value::get<int>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("int", columnName));
   return internal::getIntegral<int>("int", type, value, columnName);
}

template <>
inline long Value::get<long>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("long", columnName));
   return internal::getIntegral<long>("long", type, value, columnName);
}

template <>
inline long long Value::get<long long>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("long long", columnName));
   return internal::getIntegral<long long>("long long", type, value, columnName);
}

template <>
inline bool Value::get<bool>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("bool", columnName));
   switch (type.getTag()) {
      case TypeTag::Bool: {
         assert(value.size == sizeof(int8_t));
         int8_t v = hyper_read_int8(value.value);
         assert((v == 0) || (v == 1));
         return v != 0;
      }
      default:
         internal::throwTypeError("bool", type, columnName);
   }
}

template <>
inline double Value::get<double>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("double", columnName));
   switch (type.getTag()) {
      case TypeTag::SmallInt:
         return static_cast<double>(get<int16_t>());
      case TypeTag::Int:
         return static_cast<double>(get<int32_t>());
      case TypeTag::BigInt:
         return static_cast<double>(get<int64_t>());
      case TypeTag::Double:
         assert(value.size == sizeof(double));
         return internal::bitCast<double>(hyper_read_int64(value.value));
      case TypeTag::Numeric:
         assert(value.size == sizeof(int64_t));
         return static_cast<double>(hyper_read_int64(value.value)) / internal::tenPow[type.getScale()];
      default:
         internal::throwTypeError("double", type, columnName);
   }
}

template <>
inline long double Value::get<long double>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("double", columnName));
   return static_cast<long double>(get<double>());
}

template <class T>
inline T Value::get() const {
   return internal::ValueAccess<T>().getValue(*this);
}

template <>
inline uint32_t Value::get<uint32_t>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("uint32_t", columnName));
   switch (type.getTag()) {
      case TypeTag::Oid:
         assert(value.size == sizeof(int32_t));
         return static_cast<uint32_t>(hyper_read_int32(value.value));
      default:
         internal::throwTypeError("uint32_t", type, columnName);
   }
}

template <>
inline string_view Value::get<string_view>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("string", columnName));
   switch (type.getTag()) {
      case TypeTag::Text:
      case TypeTag::Varchar:
      case TypeTag::Char:
      case TypeTag::Json:
         return string_view(reinterpret_cast<const char*>(hyper_read_varbinary(value.value)), value.size);
      default:
         internal::throwTypeError("string", type, columnName);
   }
}

template <>
inline std::string Value::get<std::string>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("string", columnName));
   return std::string(get<string_view>());
}

template <>
inline ByteSpan Value::get<ByteSpan>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("ByteSpan", columnName));
   return {hyper_read_varbinary(value.value), value.size};
}

template <>
inline std::vector<uint8_t> Value::get<std::vector<uint8_t>>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("vector<uint8_t>", columnName));
   ByteSpan bytes = get<ByteSpan>();
   return {bytes.data, bytes.data + bytes.size};
}

template <>
inline Interval Value::get<Interval>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("Interval", columnName));
   switch (type.getTag()) {
      case TypeTag::Interval:
         assert(value.size == sizeof(hyper_data128_t));
         return Interval(hyper_read_data128(value.value), Interval::raw_t());
      default:
         internal::throwTypeError("Interval", type, columnName);
   }
}

template <>
inline Date Value::get<Date>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("Date", columnName));
   switch (type.getTag()) {
      case TypeTag::Date:
         assert(value.size == sizeof(int32_t));
         return Date(static_cast<hyper_date_t>(hyper_read_int32(value.value)), Date::raw_t());
      default:
         internal::throwTypeError("Date", type, columnName);
   }
}

template <>
inline Time Value::get<Time>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("Time", columnName));
   switch (type.getTag()) {
      case TypeTag::Time:
         assert(value.size == sizeof(int64_t));
         return Time(static_cast<hyper_time_t>(hyper_read_int64(value.value)), Time::raw_t());
      default:
         internal::throwTypeError("Time", type, columnName);
   }
}

template <>
inline Timestamp Value::get<Timestamp>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("Timestamp", columnName));
   switch (type.getTag()) {
      case TypeTag::Timestamp:
         assert(value.size == sizeof(int64_t));
         return Timestamp(static_cast<hyper_timestamp_t>(hyper_read_int64(value.value)), Timestamp::raw_t());
      default:
         internal::throwTypeError("Timestamp", type, columnName);
   }
}

template <>
inline OffsetTimestamp Value::get<OffsetTimestamp>() const {
   HYPER_PRECONDITION_MSG(value.value != nullptr, internal::createNullabilityErrorMessage("OffsetTimestamp", columnName));
   switch (type.getTag()) {
      case TypeTag::TimestampTZ:
         assert(value.size == sizeof(int64_t));
         return OffsetTimestamp(static_cast<hyper_timestamp_t>(hyper_read_int64(value.value)), OffsetTimestamp::raw_t());
      default:
         internal::throwTypeError("OffsetTimestamp", type, columnName);
   }
}

inline std::ostream& operator<<(std::ostream& os, const Value& value) {
   if (value.isNull()) {
      return os << "NULL";
   }
   switch (value.type.getTag()) {
      case TypeTag::Unsupported:
         break;
      case TypeTag::Text:
      case TypeTag::Varchar:
      case TypeTag::Char: // TODO: char1 handling should be hidden (TFSID 921664)
      case TypeTag::Json:
         return os << value.get<string_view>();
      case TypeTag::SmallInt:
         return os << value.get<short>();
      case TypeTag::Int:
         return os << value.get<int>();
      case TypeTag::BigInt:
         return os << value.get<long long>();
      case TypeTag::Bool:
         return os << value.get<bool>();
      case TypeTag::Date:
         return os << value.get<Date>();
      case TypeTag::Numeric:
         assert(value.value.size == sizeof(int64_t));
         return os << internal::numericToString(hyper_read_int64(value.value.value), value.type.getScale());
      case TypeTag::Double:
         return os << value.get<double>();
      case TypeTag::Oid:
         return os << value.get<uint32_t>();
      case TypeTag::Bytes:
      case TypeTag::Geography:
         return os << value.get<ByteSpan>();
      case TypeTag::Interval:
         return os << value.get<Interval>();
      case TypeTag::Time:
         return os << value.get<Time>();
      case TypeTag::Timestamp:
         return os << value.get<Timestamp>();
      case TypeTag::TimestampTZ:
         return os << value.get<OffsetTimestamp>();
   }
   std::abort(); // Unreachable
}

inline void Result::close() noexcept {
   if (rowset) {
      hyper_close_rowset(rowset);
      rowset = nullptr;
   }
}

inline Chunk Result::getNextChunk() {
   if (!isOpen()) {
      return Chunk();
   }
   while (true) {
      hyper_rowset_chunk_t* newChunk = nullptr;
      hyper_error_t* error = hyper_rowset_get_next_chunk(rowset, &newChunk);
      if (error) {
         throw internal::makeHyperException(error);
      }
      if (newChunk) {
         Chunk chunk(newChunk, *this);
         if (chunk.getRowCount()) {
            return chunk;
         }
      } else {
         // There are no more chunks.
         close();
         return Chunk();
      }
   }
}

inline Result::Result(hyper_rowset_t* rowset, Connection& conn)
   : rowset(rowset), conn(&conn) {
   const hyper_table_definition_t* table_def = hyper_rowset_get_table_definition(rowset);

   size_t columnCount = hyper_table_definition_column_count(table_def);
   for (hyper_field_index_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
      TypeTag tag = static_cast<TypeTag>(hyper_table_definition_column_type_tag(table_def, columnIndex));
      hyper_type_modifier_t modifier = hyper_table_definition_column_type_modifier(table_def, columnIndex);
      uint32_t oid = hyper_table_definition_column_type_oid(table_def, columnIndex);
      const char* collationPtr = hyper_table_definition_column_collation(table_def, columnIndex);
      std::string collation = collationPtr ? std::string(collationPtr) : std::string();
      std::string columnName = hyper_table_definition_column_name(table_def, columnIndex);
      ResultSchema::Column c(std::move(columnName), {tag, oid, modifier});
      schema.addColumn(std::move(c));
   }
}

inline Result::~Result() noexcept {
   close();
}

inline Result::Result(Result&& other) noexcept
   : rowset(internal::exchange(other.rowset, nullptr)), conn(other.conn), schema(std::move(other.schema)) {
}

inline Result& Result::operator=(Result&& other) noexcept {
   if (&other != this) {
      close();

      rowset = other.rowset;
      other.rowset = nullptr;
      schema = std::move(other.schema);
      conn = other.conn;
   }

   return *this;
}

inline bool Result::isOpen() const noexcept {
   return (rowset != nullptr);
}

inline optional<size_t> Result::getAffectedRowCount() const {
   HYPER_PRECONDITION_MSG(isOpen(), "Result is closed");
   int64_t rowCount = hyper_rowset_get_affected_row_count(rowset);
   if (rowCount >= 0) {
      return static_cast<size_t>(rowCount);
   }
   assert(rowCount == -1);
   return {};
}

inline Connection& Result::getConnection() {
   HYPER_PRECONDITION_MSG(isOpen(), "Result is closed");
   return *conn;
}

/** Constructor */
inline Chunk::Chunk(hyper_rowset_chunk_t* chunk, const Result& result) noexcept
   : chunk(chunk), result(&result), rowCount(hyper_rowset_chunk_row_count(chunk)) {
}

/** Destructor */
inline Chunk::~Chunk() noexcept {
   if (chunk) {
      hyper_destroy_rowset_chunk(chunk);
   }
}

/** Move constructor */
inline Chunk::Chunk(Chunk&& other) noexcept
   : chunk(internal::exchange(other.chunk, nullptr)), chunkSize(other.chunkSize), result(other.result), rowCount(other.rowCount) {
}

/** Move assignment */
inline Chunk& Chunk::operator=(Chunk&& other) noexcept {
   if (&other != this) {
      if (chunk) {
         hyper_destroy_rowset_chunk(chunk);
      }
      chunk = other.chunk;
      other.chunk = nullptr;
      chunkSize = other.chunkSize;
      result = other.result;
      rowCount = other.rowCount;
   }
   return *this;
}

inline Row Chunk::getRowAt(hyper_row_index_t chunkRowIndex) const noexcept {
   HYPER_PRECONDITION_MSG(isOpen(), "Result is closed");
   return Row(*this, chunkRowIndex);
}

inline Row::Row(const Chunk& chunk, hyper_row_index_t chunkRowIndex) noexcept
   : chunk(chunk), chunkRowIndex(chunkRowIndex) {
}

template <>
inline Value Row::get(hyper_field_index_t columnIndex) const {
   hyper_value_t value_t = hyper_rowset_chunk_field_value(chunk.get().chunk, chunkRowIndex, columnIndex);
   const ResultSchema::Column& column = getSchema().getColumn(columnIndex);
   return {value_t, column.getType(), column.getName().getUnescaped()};
}

template <class ReturnType>
inline ReturnType Row::get(hyper_field_index_t columnIndex) const {
   return get(columnIndex).get<ReturnType>();
}

inline const ResultSchema& Row::getSchema() const noexcept {
   return chunk.get().result->getSchema();
}

inline ChunkIterator::reference ChunkIterator::operator*() const noexcept {
   currentRow = chunk.get().getRowAt(rowIndex);
   return currentRow;
}

inline ChunkIterator& ChunkIterator::operator++() noexcept {
   ++rowIndex;
   return *this;
}

inline const ChunkIterator ChunkIterator::operator++(int) noexcept {
   ChunkIterator it(*this);
   ++*this;
   return it;
}

inline ColumnIterator::reference ColumnIterator::operator*() const noexcept {
   hyper_value_t internalValue = hyper_rowset_chunk_field_value(row.get().chunk.get().chunk, row.get().chunkRowIndex, columnIndex);
   const ResultSchema::Column& column = row.get().getSchema().getColumn(columnIndex);
   currentValue = {internalValue, column.getType(), column.getName().getUnescaped()};
   return currentValue;
}

inline ColumnIterator& ColumnIterator::operator++() {
   ++columnIndex;
   return *this;
}

inline const ColumnIterator ColumnIterator::operator++(int) {
   ColumnIterator it(*this);
   ++*this;
   return it;
}

inline ChunkedResultIterator& ChunkedResultIterator::operator++() {
   currentChunk = result.get().getNextChunk();
   return *this;
}

inline ResultIterator& ResultIterator::operator++() {
   ++chunkIterator;
   if (chunkIterator == end(*chunksIterator)) {
      ++chunksIterator;
      chunkIterator = begin(*chunksIterator);
   }
   return *this;
}
}

namespace std {
inline size_t hash<hyperapi::ByteSpan>::operator()(const hyperapi::ByteSpan& bs) const noexcept {
   size_t hash = 0;
   const uint8_t* bsData = bs.data;
   for (size_t index = 0; index < bs.size; ++index) {
      hash = 5 * hash + bsData[index];
   }
   return size_t(hash);
}
}
#endif
