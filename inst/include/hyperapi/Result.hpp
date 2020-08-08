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

#ifndef TABLEAU_HYPER_RESULT_HPP
#define TABLEAU_HYPER_RESULT_HPP

#include <hyperapi/ByteSpan.hpp>
#include <hyperapi/Date.hpp>
#include <hyperapi/HyperException.hpp>
#include <hyperapi/Interval.hpp>
#include <hyperapi/Numeric.hpp>
#include <hyperapi/OffsetTimestamp.hpp>
#include <hyperapi/ResultSchema.hpp>
#include <hyperapi/Time.hpp>
#include <hyperapi/Timestamp.hpp>
#include <hyperapi/string_view.hpp>
#include <hyperapi/hyperapi.h>

#include <iterator>
#include <string>

namespace hyperapi {

class ChunkIterator;
struct Chunks;
class ColumnIterator;
class Connection;
class Result;
class Row;
class ChunkedResultIterator;
class ResultIterator;

/**
  * A value inside a row. This class implicitly casts to all supported types.
  */
class Value final {
   public:
   /** Constructs a Value object. */
   Value(hyper_value_t value, SqlType type, string_view columnName) noexcept
      : value(value), type(type), columnName(columnName) {
   }

   /** Default constructs a Value object. */
   Value() noexcept {}

   /** Cast the value to one of the supported types
     *
     *  \details \copydetails get()
     */
   template <class ReturnType>
   operator ReturnType() const { return get<ReturnType>(); }

   /** Get the value as one of the supported types.
     *
     * If the value is nullable, retrieve optional<Type> instead.
     *
     * The following types are supported:
     * short
     * int
     * long
     * long long
     * bool
     * double
     * long double
     * hyperapi::Numeric<precision, scale>
     * uint32_t
     * hyperapi::string_view  // Attention: A string_view does not own its memory, so it will become invalid if the result or row go out of scope. Use std::string instead.
     * std::string
     * ByteSpan  // Attention: A ByteSpan does not own its memory, so it will become invalid if the result or row go out of scope. Use vector<const uint8_t> instead.
     * std::vector<uint8_t>
     * hyperapi::Interval
     * hyperapi::Date
     * hyperapi::Time
     * hyperapi::Timestamp
     * hyperapi::OffsetTimestamp
     *
     * \pre The requested type is compatible to the type of the value.
     */
   template <class ReturnType>
   ReturnType get() const;

   /** Get the type of the value */
   SqlType getType() const noexcept { return type; }

   /** Returns whether the value is null. */
   bool isNull() const noexcept { return !value.value; }

   /** Stream output operator */
   friend std::ostream& operator<<(std::ostream& os, const Value& value);

   /** Comparison operator */
   friend bool operator==(const Value& lhs, const Value& rhs) noexcept { return lhs.value.value == rhs.value.value; }

   /** Comparison operator */
   friend bool operator!=(const Value& lhs, const Value& rhs) noexcept { return !(lhs == rhs); }

   private:
   friend class ColumnIterator;
   template <class ReturnType>
   friend struct internal::ValueAccess;
   friend class Connection;

   /** This value has to be cast immediately. Therefore, the public interface does not support storage. */
   Value(const Value&) = default;
   Value(Value&&) = default;
   Value& operator=(const Value&) = default;
   Value& operator=(Value&&) = default;

   /// The value
   hyper_value_t value{nullptr, 0};
   /// The type of the value
   SqlType type{TypeTag::Unsupported, 0};
   /// The column name
   string_view columnName = "";
};

/**
 * A chunk of a result. A chunk is a collection of rows.
 */
class Chunk final {
   public:
   /** Constructor, constructs a closed chunk.
     *  \post !isOpen()
     */
   Chunk() noexcept = default;

   /** Destructor */
   ~Chunk() noexcept;

   /// Copy constructor
   Chunk(Chunk& other) = delete;
   /// Copy assignment
   Chunk& operator=(Chunk& other) = delete;

   /** Move constructor */
   Chunk(Chunk&& other) noexcept;
   /** Move assignment */
   Chunk& operator=(Chunk&& other) noexcept;

   /**
     * Retrieves the row at the given index inside the chunk (starting at 0).
     * \param chunkRowIndex The index of the row inside the chunk.
     * \return The row at given index.
     * \pre isOpen()
     */
   Row getRowAt(hyper_row_index_t chunkRowIndex) const noexcept;

   /**
     * \return The number of rows inside the chunk.
     * \pre isOpen()
     */
   size_t getRowCount() const noexcept { return rowCount; }

   /** Returns whether the chunk is valid. */
   bool isOpen() const noexcept { return (chunk != nullptr); }

   /** Returns whether the chunk is valid. */
   operator bool() const noexcept { return isOpen(); }

   /** Comparison operator */
   friend bool operator==(const Chunk& lhs, const Chunk& rhs) noexcept { return lhs.chunk == rhs.chunk; }

   /** Comparison operator */
   friend bool operator!=(const Chunk& lhs, const Chunk& rhs) noexcept { return !(lhs == rhs); }

   private:
   /**
     * Constructor
     *
     * \post isOpen()
     */
   Chunk(hyper_rowset_chunk_t* chunk, const Result& result) noexcept;

   /// The handle for the current chunk.
   hyper_rowset_chunk_t* chunk = nullptr;
   /// The size of the current chunk.
   size_t chunkSize = 0;
   /// The result the chunk belongs to.
   const Result* result = nullptr;
   /// The result the chunk belongs to.
   size_t rowCount = 0;

   friend class Row;
   friend class ColumnIterator;
   friend class Result;
};

/**
  * Base class for a result of a query.
  * Note: While this resource is open, the connection is busy.
  */
class Result final {
   public:
   /**
     * Constructs an empty `Result` object.
     * \post !isOpen()
     * \throw std::bad_alloc
     */
   Result() = default;

   virtual ~Result() noexcept;

   /** Move constructor */
   Result(Result&& other) noexcept;

   /** Move assignment operator */
   Result& operator=(Result&& other) noexcept;

   /// Copy is forbidden
   Result& operator=(const Result& other) = delete;
   Result(const Result& other) = delete;

   /**
     * Returns schema of the result.
     * \return The result schema.
     */
   const ResultSchema& getSchema() const noexcept { return schema; }

   /**
     * Get the affected row count, if the statement had any.
     * If this result also produced rows, then this method is only accessible after having read all rows.
     * \return the affected row count if available.
     * \pre isOpen()
     * \throw HyperException
     */
   optional<size_t> getAffectedRowCount() const;

   /**
     * Returns the connection of the SQL statement that yielded this result.
     *
     * \pre isOpen()
     */
   Connection& getConnection();

   /**
     * Checks whether the result is open. The connection is unusable as long as a result is still open.
     * \return Whether the result is open.
     */
   bool isOpen() const noexcept;

   /**
     * Closes the result. Makes the connection usable again.
     * \post !isOpen()
     */
   void close() noexcept;

   private:
   /**
     * Constructs a `result` object from the given `hyper_rowset_t*` handle.
     *
     * The `result` object takes ownership of the given handle.
     * In case of an exception, the `hyper_rowset_t*` handle is closed.
     *
     * \param rowset  The underlying `hyper_rowset_t*` handle.
     * \param conn  the connection of the SQL statement that yielded this result.
     * \post isOpen()
     */
   explicit Result(hyper_rowset_t* rowset, Connection& conn);

   /// The underlying handle.
   hyper_rowset_t* rowset = nullptr;

   /// The connection
   Connection* conn = nullptr;

   /** Get the next chunk */
   Chunk getNextChunk();

   /// The schema of the result.
   ResultSchema schema;

   friend class Row;
   friend class Connection;
   friend class Chunk;
   friend class ChunkedResultIterator;
   friend Result internal::executePreparedQuery(Connection& connection, const std::string& statement_name, hyper_rowset_result_format_t result_format);
   friend Result internal::executeQueryParams(Connection& connection, const std::string& query, hyper_rowset_result_format_t result_format);
};

/** A tag that makes a result iterable in chunks */
struct Chunks {
   /** Construct a Chunks object */
   Chunks(Result& result)
      : result(result) {
   }

   Result& result;
};

/**
  * A Row inside a chunk.
  * The row is only valid for as long as the chunk lives.
  */
class Row final {
   public:
   /**
     * Returns the value of field at position `columnIndex`.
     *
     * Example: `int a = row.get();`
     * Example: `auto a = row.get<int>();`
     * \param columnIndex The index of the field inside the row (starting at 0).
     * \return The value of type `Value`. The returned value implicitly casts to all types.
     * \see hyperapi::Value::get()
     * \pre The requested type is compatible to the column type.
     */
   template <class ReturnType = Value>
   ReturnType get(hyper_field_index_t columnIndex) const;

   /**
     * Returns the schema of the row.
     * \return The result schema.
     * \pre isOpen()
     */
   const ResultSchema& getSchema() const noexcept;

   private:
   /**
     * Constructs a row that references into a chunk at a given offset.
     * \param chunk The chunk.
     * \param chunkRowIndex The offset of the row in the chunk.
     */
   Row(const Chunk& chunk, hyper_row_index_t chunkRowIndex) noexcept;

   /** A row serves only to access the values. Therefore, the public interface does not support storage. */
   Row(const Row&) = default;
   Row(Row&&) = default;
   Row& operator=(const Row&) = default;
   Row& operator=(Row&&) = default;

   /// The associated `hyperapi::Chunk` object.
   std::reference_wrapper<const Chunk> chunk;
   /// The row's index inside the chunk
   hyper_row_index_t chunkRowIndex;

   friend class Chunk;
   friend class ResultIterator;
   friend class ColumnIterator;
   friend class ChunkIterator;
};

/** A tag for an iterator-begin constructor */
struct IteratorBeginTag {
};
static constexpr IteratorBeginTag iteratorBeginTag = {};

/** A tag for an iterator-end constructor */
struct IteratorEndTag {
};
static constexpr IteratorEndTag iteratorEndTag = {};

/** Iterates over a `hyperapi::Chunk` in rows (`hyperapi::Row`). */
class ChunkIterator final {
   public:
   using iterator_category = std::input_iterator_tag;
   using value_type = Row;
   using difference_type = std::ptrdiff_t;
   using pointer = const Row*;
   using reference = const Row&;

   /** Constructs the begin-iterator. */
   ChunkIterator(const Chunk& chunk, IteratorBeginTag) noexcept
      : chunk(chunk), rowIndex(0), currentRow(chunk, 0) {
   }

   /** Constructs the end-iterator. */
   ChunkIterator(const Chunk& chunk, IteratorEndTag) noexcept
      : chunk(chunk), rowIndex(static_cast<hyper_row_index_t>(chunk.getRowCount())), currentRow(chunk, 0) {
   }

   /** Returns a reference to the current value. */
   reference operator*() const noexcept;

   /** Returns a pointer to the current value. */
   pointer operator->() const noexcept { return &**this; }

   /** Advances the iterator. */
   ChunkIterator& operator++() noexcept;

   /** Advances the iterator. */
   const ChunkIterator operator++(int) noexcept;

   /** Comparison operator */
   friend bool operator==(const ChunkIterator& lhs, const ChunkIterator& rhs) noexcept { return (lhs.chunk.get() == rhs.chunk.get()) && (lhs.rowIndex == rhs.rowIndex); }

   /** Comparison operator */
   friend bool operator!=(const ChunkIterator& lhs, const ChunkIterator& rhs) noexcept { return !(lhs == rhs); }

   private:
   std::reference_wrapper<const Chunk> chunk;
   hyper_row_index_t rowIndex;
   mutable Row currentRow;
};

/** Returns the begin-iterator for the rows of the given chunk. */
inline ChunkIterator begin(const Chunk& chunk) {
   return {chunk, iteratorBeginTag};
}

/** Returns the end-iterator for the rows of the given chunk. */
inline ChunkIterator end(const Chunk& chunk) noexcept {
   return {chunk, iteratorEndTag};
}

/** Iterates over a `hyperapi::Row` in values (`hyperapi::Value`). */
class ColumnIterator final {
   public:
   using iterator_category = std::input_iterator_tag;
   using value_type = Value;
   using difference_type = std::ptrdiff_t;
   using pointer = const Value*;
   using reference = const Value&;

   /** Constructs the begin-iterator. */
   ColumnIterator(const Row& row, IteratorBeginTag) noexcept
      : row(row), columnIndex(0) {
   }

   /** Constructs the end-iterator. */
   ColumnIterator(const Row& row, IteratorEndTag) noexcept
      : row(row), columnIndex(static_cast<hyper_field_index_t>(row.getSchema().getColumnCount())) {
   }

   /** Returns a reference to the current value. */
   reference operator*() const noexcept;

   /** Returns a pointer to the current value. */
   pointer operator->() const noexcept { return &**this; }

   /** Advances the iterator. */
   ColumnIterator& operator++();

   /** Advances the iterator. */
   const ColumnIterator operator++(int);

   /** Comparison operator */
   friend bool operator==(const ColumnIterator& lhs, const ColumnIterator& rhs) noexcept { return (&lhs.row.get() == &rhs.row.get()) && (lhs.columnIndex == rhs.columnIndex); }

   /** Comparison operator */
   friend bool operator!=(const ColumnIterator& lhs, const ColumnIterator& rhs) noexcept { return !(lhs == rhs); }

   private:
   std::reference_wrapper<const Row> row;
   hyper_field_index_t columnIndex;
   mutable Value currentValue;
};

/** Returns the begin-iterator for the values of the given row. */
inline ColumnIterator begin(const Row& row) {
   return {row, iteratorBeginTag};
}

/** Returns the end-iterator for the values of the given row. */
inline ColumnIterator end(const Row& row) noexcept {
   return {row, iteratorEndTag};
}

/** Iterates over a `hyperapi::Result` in `hyperapi::Chunk`. */
class ChunkedResultIterator final {
   public:
   using iterator_category = std::input_iterator_tag;
   using value_type = Chunk;
   using difference_type = std::ptrdiff_t;
   using pointer = Chunk*;
   using reference = Chunk&;

   /** Constructs the begin-iterator. */
   ChunkedResultIterator(Result& result, IteratorBeginTag) noexcept
      : result(result), currentChunk(result.getNextChunk()) {
   }

   /** Constructs the end-iterator. */
   ChunkedResultIterator(Result& result, IteratorEndTag) noexcept
      : result(result), currentChunk() {
   }

   /** Returns a reference to the current value. */
   reference operator*() noexcept { return currentChunk; }

   /** Returns a pointer to the current value. */
   pointer operator->() noexcept { return &**this; }

   /** Advances the iterator. */
   ChunkedResultIterator& operator++();

   /** Comparison operator */
   friend bool operator==(const ChunkedResultIterator& lhs, const ChunkedResultIterator& rhs) noexcept { return lhs.currentChunk == rhs.currentChunk; }

   /** Comparison operator */
   friend bool operator!=(const ChunkedResultIterator& lhs, const ChunkedResultIterator& rhs) noexcept { return !(lhs == rhs); }

   private:
   std::reference_wrapper<Result> result;
   Chunk currentChunk;
};

/** Returns the begin-iterator for the chunks of the given result. */
inline ChunkedResultIterator begin(const Chunks& chunks) {
   return {chunks.result, iteratorBeginTag};
}

/** Returns the end-iterator for the chunks of the given result. */
inline ChunkedResultIterator end(const Chunks& chunks) noexcept {
   return {chunks.result, iteratorEndTag};
}

/**
 * Iterates over a `hyperapi::Result` in rows (`hyperapi::Row`).
 *
 * `ResultIterator` is a single-pass input iterator.
 * It reads successive rows of type `hyperapi::Row` from the `hyperapi::Result` from which it was constructed.
 */
class ResultIterator final {
   public:
   using iterator_category = std::input_iterator_tag;
   using value_type = Row;
   using difference_type = std::ptrdiff_t;
   using pointer = const Row*;
   using reference = const Row&;

   /** Constructs the begin-iterator. */
   ResultIterator(Result& result, IteratorBeginTag) noexcept
      : chunksIterator(begin(Chunks(result))), chunkIterator(begin(*chunksIterator)) {
   }

   /** Constructs the end-iterator. */
   ResultIterator(Result& result, IteratorEndTag) noexcept
      : chunksIterator(end(Chunks(result))), chunkIterator(end(*chunksIterator)) {
   }

   /** Returns a reference to the current value. */
   reference operator*() const noexcept { return *chunkIterator; }

   /** Returns a pointer to the current value. */
   pointer operator->() const noexcept { return &**this; }

   /** Advances the iterator. */
   ResultIterator& operator++();

   /** Comparison operator */
   friend bool operator==(const ResultIterator& lhs, const ResultIterator& rhs) noexcept { return lhs.chunkIterator == rhs.chunkIterator; }

   /** Comparison operator */
   friend bool operator!=(const ResultIterator& lhs, const ResultIterator& rhs) noexcept { return !(lhs == rhs); }

   private:
   ChunkedResultIterator chunksIterator;
   ChunkIterator chunkIterator;
};

/** Returns the begin-iterator for the rows of the given result. */
inline ResultIterator begin(Result& result) {
   return {result, iteratorBeginTag};
}

/** Returns the end-iterator for the rows of the given result. */
inline ResultIterator end(Result& result) noexcept {
   return {result, iteratorEndTag};
}
}

#include <hyperapi/impl/Result.impl.hpp>

#endif
