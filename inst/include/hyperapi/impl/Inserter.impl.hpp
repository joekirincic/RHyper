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

#ifndef TABLEAU_HYPER_IMPL_INSERTER_IMPL_HPP
#define TABLEAU_HYPER_IMPL_INSERTER_IMPL_HPP

#include <hyperapi/ByteSpan.hpp>
#include <hyperapi/hyperapi.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <hyperapi/Catalog.hpp>
#include <iostream> // TODO: replace with logging
#include <string>

namespace hyperapi {

namespace internal {
/**
 * Return an altered version of the given `original` table definition. This altered version contains a
 * rearranged subset of columns as specified by the given `columns` parameter.
 * \param columns  The desired columns in the desired order.
 * \return An altered version of the `original` table definition.
 * \pre The `original` table definition contains all given columns.
 */
inline static TableDefinition alterTableDefinition(const TableDefinition& original, const std::vector<std::string> columns) {
   TableDefinition tableDefinition(original.getTableName(), original.getPersistence());

   for (const std::string& col : columns) {
      const TableDefinition::Column* originalCol = original.getColumnByName(col);
      HYPER_PRECONDITION_MSG(originalCol != nullptr, "Given column not part of the given table definition.");
      tableDefinition.addColumn(TableDefinition::Column(*originalCol));
   }
   return tableDefinition;
}
/**
 * Return an altered version of the given `original` table definition using the given `columnMappings`.
 * This altered version contains a rearranged subset of columns as specified by the given `columnMappings`.
 * \param original The original table definition
 * \param columnMappings The desired columns in the desired order.
 * \return An altered version of the `original` table definition.
 * \pre The `original` table definition contains all given columnMappings.
 */
inline static TableDefinition alterTableDefinitionFromColumnMappings(const TableDefinition& original, const std::vector<Inserter::ColumnMapping> columnMappings) {
   TableDefinition tableDefinition(original.getTableName(), original.getPersistence());

   for (const Inserter::ColumnMapping& col : columnMappings) {
      const TableDefinition::Column* originalCol = original.getColumnByName(col.getColumnName());
      HYPER_PRECONDITION_MSG(originalCol != nullptr, "Given column not part of the given table definition.");
      tableDefinition.addColumn(TableDefinition::Column(*originalCol));
   }
   return tableDefinition;
}
}

inline void Inserter::newChunk() noexcept {
   // Write the header into the new chunk and reset the chunk usage
   headerSize = hyper_write_header(currentChunk.data(), currentChunk.size());
   assert(chunkOffset <= currentChunk.size());
   chunkOffset = headerSize;
}

inline Inserter::ColumnMapping::ColumnMapping(Name name)
   : name(std::move(name)), expression({}) {
}

inline Inserter::ColumnMapping::ColumnMapping(Name name, std::string expression)
   : name(std::move(name)), expression(std::move(expression)) {
}

inline std::string Inserter::ColumnMapping::asSelectListExpression() const {
   if (expression) {
      return expression.value() + " AS " + name.toString();
   } else {
      return name.toString();
   }
}

inline Inserter::Inserter(hyperapi::Connection& connection, const hyperapi::TableDefinition tableDefinition)
   : tableDefinition(std::move(tableDefinition)), tableDefinitionHandle(this->tableDefinition), streamDefinition(this->tableDefinition), streamDefinitionHandle(this->streamDefinition) {
   currentChunk.resize(1024 * 1024);

   if (hyper_error_t* error = hyper_create_inserter(connection.m_handle, tableDefinitionHandle.get(), &inserter))
      throw internal::makeHyperException(error);

   // Build the select for bulk inserter
   std::ostringstream selectListStream;
   const std::vector<TableDefinition::Column>& columns = tableDefinition.getColumns();
   if (!columns.empty()) {
      for (auto it = columns.begin();;) {
         selectListStream << it->getName().toString();
         if (++it == columns.end())
            break;
         selectListStream << ", ";
      }
      selectList = selectListStream.str();
   }
   newChunk();
}

inline Inserter::Inserter(Connection& connection, const hyperapi::TableDefinition& tableDefinition, std::vector<Inserter::ColumnMapping> columnMappings, std::vector<TableDefinition::Column> inserterDefinition)
   : tableDefinition(internal::alterTableDefinitionFromColumnMappings(tableDefinition, columnMappings)), tableDefinitionHandle(this->tableDefinition), streamDefinition(this->tableDefinition.getTableName(), inserterDefinition, this->tableDefinition.getPersistence()), streamDefinitionHandle(this->streamDefinition) {
   HYPER_PRECONDITION_MSG(!columnMappings.empty(), "Column Mappings cannot be empty");
   currentChunk.resize(1024 * 1024);

   // Input values for non-expression columns must be provided by the user
   // Verify that all non-expression columns are specified in the inserter definition
   // Verify the definition of all non-expression columns provided in the inserter definition match the actual column definitions
   const std::vector<TableDefinition::Column>& targetColumns = this->tableDefinition.getColumns();
   auto iter = begin(targetColumns);
   for (const auto& columnMapping : columnMappings) {
      if (!columnMapping.getExpression()) {
         const TableDefinition::Column* inserterColumn = this->streamDefinition.getColumnByName(columnMapping.getColumnName());
         HYPER_PRECONDITION_MSG(inserterColumn != nullptr, "Given column not part of the given inserter definition.");

         assert(iter != end(targetColumns));
         const TableDefinition::Column& targetColumn = *iter;

         HYPER_PRECONDITION_MSG(targetColumn.getType() == inserterColumn->getType() && targetColumn.getNullability() == inserterColumn->getNullability(),
                                "Column definition for " + columnMapping.getColumnName().toString() + " does not match the definition provided in the inserter definition");
      }
      iter++;
   }

   if (hyper_error_t* error = hyper_create_inserter(connection.m_handle, tableDefinitionHandle.get(), &inserter))
      throw internal::makeHyperException(error);

   // Build the select for bulk inserter
   std::ostringstream selectListStream;
   for (auto it = columnMappings.begin();;) {
      selectListStream << it->asSelectListExpression();
      if (++it == columnMappings.end())
         break;
      selectListStream << ", ";
   }
   selectList = selectListStream.str();

   // Initialize Bulk Insert to support expressions during insertion
   if (hyper_error_t* error = hyper_init_bulk_insert(inserter, streamDefinitionHandle.get(), selectList.c_str())) {
      close();
      throw internal::makeHyperException(error);
   }

   newChunk();
}

inline Inserter::Inserter(Connection& connection, const TableName& name, std::vector<Inserter::ColumnMapping> columnMappings, std::vector<TableDefinition::Column> inserterDefinition)
   : Inserter(connection, connection.getCatalog().getTableDefinition(name), columnMappings, inserterDefinition) {
}

inline Inserter::Inserter(Connection& connection, const hyperapi::TableDefinition& tableDefinition, std::vector<std::string> columns)
   : Inserter(connection, internal::alterTableDefinition(tableDefinition, columns)) {
}

inline Inserter::Inserter(Connection& connection, const TableName& name)
   : Inserter(connection, connection.getCatalog().getTableDefinition(name)) {
}

inline Inserter::Inserter(Connection& connection, const TableName& name, std::vector<std::string> columns)
   : Inserter(connection, internal::alterTableDefinition(connection.getCatalog().getTableDefinition(name), columns)) {
}

inline Inserter::~Inserter() noexcept {
   close();
}

inline Inserter::Inserter(Inserter&& other) noexcept
   : tableDefinition(std::move(other.tableDefinition)), tableDefinitionHandle(std::move(other.tableDefinitionHandle)), streamDefinition(std::move(other.streamDefinition)), streamDefinitionHandle(std::move(other.streamDefinitionHandle)) {
   inserter = other.inserter;
   other.inserter = nullptr;
   currentChunk = std::move(other.currentChunk);
   chunkOffset = other.chunkOffset;
   headerSize = other.headerSize;
   currentField = other.currentField;
}

inline Inserter& Inserter::operator=(Inserter&& other) noexcept {
   if (this != &other) {
      close();
      tableDefinition = std::move(other.tableDefinition);
      tableDefinitionHandle = std::move(other.tableDefinitionHandle);
      inserter = other.inserter;
      other.inserter = nullptr;
      currentChunk = std::move(other.currentChunk);
      chunkOffset = other.chunkOffset;
      headerSize = other.headerSize;
      currentField = other.currentField;
   }
   return *this;
}

inline bool Inserter::isOpen() const noexcept {
   return inserter != nullptr;
}

inline void Inserter::close() noexcept {
   if (!inserter)
      return;

   if (hyper_error_t* error = hyper_close_inserter(inserter, false)) {
      // Ignore the error
      internal::makeHyperException(error);
   }

   inserter = nullptr;
}

namespace internal {
template <class ValueType>
inline size_t writeIntegral(uint8_t* target, size_t space, ValueType value) noexcept {
   static_assert((sizeof(ValueType) == 2) || (sizeof(ValueType) == 4) || (sizeof(ValueType) == 8), "unexpected `sizeof(ValueType)`");
   switch (sizeof(ValueType)) {
      case 2: return hyper_write_int16(target, space, static_cast<int16_t>(value));
      case 4: return hyper_write_int32(target, space, static_cast<int32_t>(value));
      case 8: return hyper_write_int64(target, space, static_cast<int64_t>(value));
   }
   HYPER_UNREACHABLE();
}

template <class ValueType>
inline size_t writeIntegralNotNullable(uint8_t* target, size_t space, ValueType value) noexcept {
   static_assert((sizeof(ValueType) == 2) || (sizeof(ValueType) == 4) || (sizeof(ValueType) == 8), "unexpected `sizeof(ValueType)`");
   switch (sizeof(ValueType)) {
      case 2: return hyper_write_int16_not_null(target, space, static_cast<int16_t>(value));
      case 4: return hyper_write_int32_not_null(target, space, static_cast<int32_t>(value));
      case 8: return hyper_write_int64_not_null(target, space, static_cast<int64_t>(value));
   }
   HYPER_UNREACHABLE();
}

struct ValueInserter {
   const hyperapi::TableDefinition::Column& column;
   std::vector<uint8_t>& currentChunk;
   size_t& chunkOffset;
   hyper_field_index_t& currentField;

   ValueInserter(Inserter& inserter)
      : column(inserter.streamDefinition.getColumn(inserter.currentField)),
        currentChunk(inserter.currentChunk),
        chunkOffset(inserter.chunkOffset),
        currentField(inserter.currentField) {}

   template <class ValueType>
   inline void addValue(ValueType value) {
      size_t remainingSize = currentChunk.size() - chunkOffset;
      size_t requiredSize = write(column.getNullability(), currentChunk.data() + chunkOffset, remainingSize, value);

      if (requiredSize > remainingSize) {
         currentChunk.resize(std::max(currentChunk.size() + requiredSize, currentChunk.size() * 2));
         return add(value);
      }
      chunkOffset += requiredSize;
      ++currentField;
   }

   inline void addNull() {
      size_t remainingSize = currentChunk.size() - chunkOffset;
      size_t requiredSize = hyper_write_null(currentChunk.data() + chunkOffset, remainingSize);

      if (requiredSize > remainingSize) {
         currentChunk.resize(std::max(currentChunk.size() + requiredSize, currentChunk.size() * 2));
         return addNull();
      }
      chunkOffset += requiredSize;
      ++currentField;
   }

   template <class ValueType>
   [[noreturn]] inline void throwTypeViolation() {
      SqlType requestedType = internal::getSqlType<ValueType>();
      SqlType actualType = column.getType();
      HYPER_PRECONDITION_MSG(actualType.getTag() == requestedType.getTag(),
                             "Wrong add method used: Inserter::add(" + internal::getNativeTypeName(requestedType) + ") cannot be used for field #" +
                                std::to_string(currentField) + " which is of type " + actualType.toString() + ".");
      std::abort(); // unreachable
   }

   template <unsigned precision, unsigned scale>
   inline void add(Numeric<precision, scale> value) {
      switch (column.getType().getTag()) {
         case TypeTag::Numeric:
            return addValue(internal::cast(value.m_value, precision, scale, precision, scale));
         case TypeTag::Double:
            return addValue(static_cast<double>(value.m_value) / internal::tenPow[scale]);
         default: throwTypeViolation<Numeric<precision, scale>>();
      }
   }

   inline void add(string_view value) {
      switch (column.getType().getTag()) {
         case TypeTag::Text:
         case TypeTag::Json:
         case TypeTag::Char:
         case TypeTag::Varchar:
            return addValue(value);
         default: throwTypeViolation<string_view>();
      }
   }

   template <class ValueType>
   inline void addIntegral(ValueType value) {
      switch (column.getType().getTag()) {
         case TypeTag::SmallInt:
            if (sizeof(ValueType) == 2) {
               return addValue(static_cast<int16_t>(value));
            }
            throwTypeViolation<ValueType>();
         case TypeTag::Int:
            if (sizeof(ValueType) <= 4) {
               return addValue(static_cast<int32_t>(value));
            }
            throwTypeViolation<ValueType>();
         case TypeTag::BigInt:
            return addValue(static_cast<int64_t>(value));
         case TypeTag::Numeric:
            return addValue(internal::cast(Numeric<18, 0>(value).m_value, 18, 0, column.getType().getPrecision(), column.getType().getScale()));
         case TypeTag::Double:
            return addValue<double>(static_cast<double>(value));
         default: throwTypeViolation<ValueType>();
      }
   }

   inline void add(short value) {
      addIntegral<short>(value);
   }

   inline void add(int value) {
      addIntegral<int>(value);
   }

   inline void add(long value) {
      addIntegral<long>(value);
   }

   inline void add(long long value) {
      addIntegral<long long>(value);
   }

   inline void add(bool value) {
      switch (column.getType().getTag()) {
         case TypeTag::Bool:
            return addValue(value);
         default: throwTypeViolation<bool>();
      }
   }

   inline void add(const char* value) {
      add(string_view(value));
   }

   inline void add(double value) {
      switch (column.getType().getTag()) {
         case TypeTag::Double:
            return addValue(value);
         default: throwTypeViolation<double>();
      }
   }

   inline void add(uint32_t value) {
      switch (column.getType().getTag()) {
         case TypeTag::Oid:
            return addValue(value);
         default: throwTypeViolation<uint32_t>();
      }
   }

   inline void add(Interval value) {
      switch (column.getType().getTag()) {
         case TypeTag::Interval:
            return addValue(value);
         default: throwTypeViolation<Interval>();
      }
   }

   inline void add(Date value) {
      switch (column.getType().getTag()) {
         case TypeTag::Date:
            return addValue(value);
         default: throwTypeViolation<Date>();
      }
   }

   inline void add(Timestamp value) {
      switch (column.getType().getTag()) {
         case TypeTag::Timestamp:
            return addValue(value);
         default: throwTypeViolation<Timestamp>();
      }
   }

   inline void add(OffsetTimestamp value) {
      switch (column.getType().getTag()) {
         case TypeTag::TimestampTZ:
            return addValue(value);
         default: throwTypeViolation<OffsetTimestamp>();
      }
   }

   inline void add(Time value) {
      switch (column.getType().getTag()) {
         case TypeTag::Time:
            return addValue(value);
         default: throwTypeViolation<Time>();
      }
   }

   inline void add(ByteSpan value) {
      addValue(value);
   }

   template <class ValueType>
   inline void add(optional<ValueType> value) {
      if (value.has_value()) {
         return add(*value);
      } else {
         addNull();
      }
   }

   [[noreturn]] inline void add(internal::AnyType) {
      std::abort(); // unreachable
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, bool value) noexcept {
      return (nullability == Nullability::Nullable) ?
         hyper_write_int8(target, space, value) :
         hyper_write_int8_not_null(target, space, value);
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, short value) noexcept {
      return (nullability == Nullability::Nullable) ?
         writeIntegral(target, space, value) :
         writeIntegralNotNullable(target, space, value);
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, int value) noexcept {
      return (nullability == Nullability::Nullable) ?
         writeIntegral(target, space, value) :
         writeIntegralNotNullable(target, space, value);
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, long value) noexcept {
      return (nullability == Nullability::Nullable) ?
         writeIntegral(target, space, value) :
         writeIntegralNotNullable(target, space, value);
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, long long value) noexcept {
      return (nullability == Nullability::Nullable) ?
         writeIntegral(target, space, value) :
         writeIntegralNotNullable(target, space, value);
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, double value) noexcept {
      return (nullability == Nullability::Nullable) ?
         writeIntegral(target, space, bitCast<int64_t>(value)) :
         writeIntegralNotNullable(target, space, bitCast<int64_t>(value));
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, uint32_t value) noexcept {
      return (nullability == Nullability::Nullable) ?
         writeIntegral(target, space, static_cast<int32_t>(value)) :
         writeIntegralNotNullable(target, space, static_cast<int32_t>(value));
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, ByteSpan value) noexcept {
      return (nullability == Nullability::Nullable) ?
         hyper_write_varbinary(target, space, value.data, value.size) :
         hyper_write_varbinary_not_null(target, space, value.data, value.size);
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, string_view value) noexcept {
      return (nullability == Nullability::Nullable) ?
         hyper_write_varbinary(target, space, reinterpret_cast<const uint8_t*>(value.data()), value.size()) :
         hyper_write_varbinary_not_null(target, space, reinterpret_cast<const uint8_t*>(value.data()), value.size());
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, Interval value) noexcept {
      return (nullability == Nullability::Nullable) ?
         hyper_write_data128(target, space, value.getRaw()) :
         hyper_write_data128_not_null(target, space, value.getRaw());
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, Time value) noexcept {
      return (nullability == Nullability::Nullable) ?
         writeIntegral(target, space, static_cast<int64_t>(value.getRaw())) :
         writeIntegralNotNullable(target, space, static_cast<int64_t>(value.getRaw()));
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, Date value) noexcept {
      return (nullability == Nullability::Nullable) ?
         writeIntegral(target, space, static_cast<int32_t>(value.getRaw())) :
         writeIntegralNotNullable(target, space, static_cast<int32_t>(value.getRaw()));
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, Timestamp value) noexcept {
      return (nullability == Nullability::Nullable) ?
         writeIntegral(target, space, static_cast<int64_t>(value.getRaw())) :
         writeIntegralNotNullable(target, space, static_cast<int64_t>(value.getRaw()));
   }

   inline size_t write(Nullability nullability, uint8_t* target, size_t space, OffsetTimestamp value) noexcept {
      return (nullability == Nullability::Nullable) ?
         writeIntegral(target, space, static_cast<int64_t>(value.getRaw())) :
         writeIntegralNotNullable(target, space, static_cast<int64_t>(value.getRaw()));
   }
};
}

template <class ValueType>
Inserter& Inserter::add(ValueType value) {
   HYPER_PRECONDITION_MSG(isOpen(), "The inserter is closed.");
   HYPER_PRECONDITION_MSG(currentField < streamDefinition.getColumnCount(), "Inserter::add() was called for a complete row.");
   auto& column = streamDefinition.getColumn(currentField);
   HYPER_PRECONDITION_MSG((column.getNullability() == Nullability::Nullable) || (internal::getNullability<ValueType>() == Nullability::NotNullable),
                          std::string("Cannot insert a NULL value into column #") + std::to_string(currentField) + ", which has a NOT NULL constraint.");
   internal::ValueInserter(*this).add(value);
   return *this;
}

namespace internal {
template <class ValueType>
void addRow(Inserter& inserter, ValueType value) {
   inserter.add(std::forward<ValueType>(value));
}

template <class ValueType, class... ValueTypes>
void addRow(Inserter& inserter, ValueType value, ValueTypes... values) {
   addRow(inserter, std::forward<ValueType>(value));
   addRow(inserter, std::forward<ValueTypes>(values)...);
}
}

template <class... ValueTypes>
Inserter& Inserter::addRow(ValueTypes... values) {
   internal::addRow(*this, std::forward<ValueTypes>(values)...);
   endRow();
   return *this;
}

inline void Inserter::sendChunk() {
   if (hyper_error_t* error = hyper_inserter_insert_chunk(inserter, currentChunk.data(), chunkOffset)) {
      close();
      throw internal::makeHyperException(error);
   }

   newChunk();
}

inline void Inserter::execute() {
   HYPER_PRECONDITION_MSG(isOpen(), "The inserter is closed.");

   if (currentField != 0) {
      // Execute() will always close the inserter. We therefore also close on precondition violation here.
      close();
      HYPER_PRECONDITION_MSG((currentField == 0), "`endRow()` wasn't called for the last row at the time Inserter::execute() was called.");
   }

   if (streamDefinition.getColumnCount() == 0) {
      // No input values were provided by the user
      // Data to be inserted is computed by the expressions
      // For example `INSERT INTO table(A) SELECT generate_series(1,10)`
      if (hyper_error_t* error = hyper_insert_computed_expressions(inserter, selectList.c_str())) {
         close();
         throw internal::makeHyperException(error);
      }
   } else {
      if (chunkOffset > headerSize) {
         sendChunk();
      }
   }

   if (hyper_error_t* error = hyper_close_inserter(inserter, true)) {
      inserter = nullptr;
      throw internal::makeHyperException(error);
   }

   inserter = nullptr;
}

inline Inserter& Inserter::endRow() {
   HYPER_PRECONDITION_MSG(isOpen(), "The `Inserter` is closed.");
   const size_t columnCount = streamDefinition.getColumnCount();
   HYPER_PRECONDITION_MSG(
      (currentField == columnCount), "`Inserter::endRow()` was called for an incomplete row with " + std::to_string(currentField) + " values. The table has " + std::to_string(columnCount) + " columns.");

   // Insert the chunk on a row boundary if a certain size is reached
   constexpr size_t CHUNK_LIMIT = 15 * 1024 * 1024;

   if (chunkOffset >= CHUNK_LIMIT) {
      if (hyper_error_t* error = hyper_init_bulk_insert(inserter, streamDefinitionHandle.get(), selectList.c_str())) {
         close();
         throw internal::makeHyperException(error);
      }

      sendChunk();
   }

   currentField = 0;
   return *this;
}

namespace internal {
/**
 * Copy the given buffer.
 * \param connection The connection.
 * \param buffer A buffer containing the data that will be sent.
 * \param size The size in bytes of the buffer that will be sent.
 */
void copyData(hyperapi::Connection& connection, const uint8_t* buffer, size_t size);

/**
 * End the copy statement.
 * \param connection The connection.
 */
void copyEnd(hyperapi::Connection& connection);

inline void copyData(hyperapi::Connection& connection, const uint8_t* value, size_t len) {
   hyper_error_t* error = hyper_copy_data(getHandle(connection), value, static_cast<int>(len));
   if (error) {
      throw internal::makeHyperException(error);
   }
}

inline void copyEnd(hyperapi::Connection& connection) {
   hyper_error_t* error = hyper_copy_end(getHandle(connection));
   if (error) {
      throw internal::makeHyperException(error);
   }
}
}
}

#endif
