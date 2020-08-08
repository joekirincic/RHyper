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

#ifndef TABLEAU_HYPER_INSERTER_HPP
#define TABLEAU_HYPER_INSERTER_HPP

#include <hyperapi/Connection.hpp>
#include <hyperapi/TableDefinition.hpp>
#include <hyperapi/Time.hpp>
#include <hyperapi/optional.hpp>
#include <hyperapi/hyperapi.h>

#include <string>
#include <vector>

namespace hyperapi {

/**
 * An inserter. Used to insert data into existing tables in Hyper.
 * The insertion happens row by row. Inside one row, all the columns have to be added sequentially in the right order.
 * Note: While this resource is open, the connection is busy.
 */
class Inserter final {
   public:
   /** Maps an expression to a column   */
   class ColumnMapping final {
      public:
      /**
        * Returns the name of the column.
        */
      const Name& getColumnName() const noexcept { return name; }

      /**
        * Returns the expression mapped to the column.
        */
      optional<std::string> getExpression() const noexcept { return expression; }

      /**
        * Creates a column mapping.
        *
        * \param name  The name
        */
      ColumnMapping(Name name);

      /**
        * Creates a column mapping.
        *
        * \param name  The name
        * \param expression The expression
        */
      ColumnMapping(Name name, std::string expression);

      private:
      /// The name
      const Name name;
      /// The optional expression
      const optional<std::string> expression;
      /** \returns Returns the properly quoted and escaped string representation of this column mapping. */
      std::string asSelectListExpression() const;

      friend class Inserter;
   };

   /**
     * Creates an inserter on a table.
     * \param connection  The connection to the Hyper instance containing the table.
     * \param name  The name of the table.
     * \throws HyperException
     * \pre connection.isOpen()
     * \post isOpen()
     */
   Inserter(Connection& connection, const TableName& name);

   /**
     * Creates an inserter on a table.
     * \param connection  The connection to the Hyper instance containing the table.
     * \param name  The name of the table.
     * \param columns  The set of columns in which to insert. The columns have to exist in the table.
     * \throws HyperException
     * \pre connection.isOpen()
     * \pre all given columns are part of the table definition
     * \post isOpen()
     */
   Inserter(Connection& connection, const TableName& name, std::vector<std::string> columns);

   /**
     * Creates an inserter on a table.
     * Note: Will not keep a reference to the table definition. You can modify it afterwards.
     * \param connection  The connection to the Hyper instance containing the table.
     * \param tableDefinition  The table definition for the table into which the data is inserted.
     * \throws HyperException
     * \pre connection.isOpen()
     * \post isOpen()
     */
   Inserter(Connection& connection, hyperapi::TableDefinition tableDefinition);

   /**
     * Creates an inserter on a table.
     * Note: Will not keep a reference to the table definition. You can modify it afterwards.
     * \param connection  The connection to the Hyper instance containing the table.
     * \param tableDefinition  The table definition for the table into which the data is inserted.
     * \param columns  The set of columns into which to insert. The columns have to be contained in the
     * `table_definition`. \throws HyperException \pre connection.isOpen() \pre all given columns are part of the table
     * definition \post isOpen()
     */
   Inserter(Connection& connection, const hyperapi::TableDefinition& tableDefinition, std::vector<std::string> columns);

   /**
     * Creates an inserter for an existing table.
     * Note: Will not keep a reference to the table definition. You can modify it afterwards.
     * Note: SQL expression provided during insertion are used without any modification during insertion and hence vulnerable to SQL injection attacks.
     * Applications must prevent end-users from providing expressions directly during insertion
     *
     * \param connection The connection to the Hyper instance containing the table.
     * \param tableDefinition The name of the table.
     * \param columnMappings  The set of columns in which to insert. The columns have to exist in the table. The columnMappings cannot be empty.
     *  Columns not present in columMappings will be set to their default value.
     *  A columnMapping can optionally contain a valid SQL expression.
     *  The SQL expression specified for the column is used to transform or compute values on the fly during insertion.
     *  The SQL expression can depend on columns that exist in the table.
     *  The SQL expression can also depend on values or columns that do not exist in the table, but must be specified in the `inserterDefinition`.
     * \param inserterDefinition The definition of columns to which values are provided.
     *  The column definition for all the columns without SQL expression must be specified in `inserterDefinition`.
     *  For a column without SQL expression the column definition provided in `inserterDefinition` must match the actual definition of the column in the table.
     *  All columns used by SQL expressions specified in `columnMappings` must be in `inserterDefinition`.
     * \throws HyperException
     * \pre connection.isOpen()
     * \pre all columns in columnMappings are part of the table definition
     * \pre columns without expressions are provided in the inserter definition
     * \post isOpen()

     * Consider the following pseudo-code on how to transform or compute values on the fly during insertion:
     * TableDefinition    : [TableName="example", Columns=["ColumnA" as INT, "ColumnB" as BIG_INT]]
     * ColumnMapping      : [[Name: "ColumnA"], [Name:"ColumnB", Expression:"ColumnA"*"ColumnC"]]
     * InserterDefinition : ["ColumnA" integer, "ColumnC" integer]
     *
     * Notice that "ColumnA" does not specify an expression and "ColumnB" is a product of "ColumnA" and "ColumnC", but "ColumnC" is not part of the table.
     * The InserterDefinition contains "ColumnA" and "ColumnC"
     *   "ColumnA" since it is not computed on the fly and has to be provided to the inserter
     *   "ColumnC" since it is specified in the SQL expression that computes "ColumnB" on the fly
     *
     * try (Inserter inserter(conn, "example", columnMapping, inserterDefinition)) {
     *      inserter.add(2).add(3).endRow();
     *      inserter.execute();
     * }
     * The insertion code snippet above inserts 2 into "ColumnA" and 6 into "ColumnB" (product of 2 and 3)
     */
   Inserter(Connection& connection, const hyperapi::TableDefinition& tableDefinition, std::vector<Inserter::ColumnMapping> columnMappings, std::vector<TableDefinition::Column> inserterDefinition);

   /**
     * Creates an inserter for an existing table.
     * Note: Will not keep a reference to the table definition. You can modify it afterwards.
     * Note: SQL expression provided during insertion are used without any modification during insertion and hence vulnerable to SQL injection attacks.
     * Applications must prevent end-users from providing expressions directly during insertion
     *
     * \param connection The connection to the Hyper instance containing the table.
     * \param name  The name of the table.
     * \param columnMappings  The set of columns in which to insert. The columns have to exist in the table. The columnMappings cannot be empty.
     *  Columns not present in columMappings will be set to their default value.
     *  A columnMapping can optionally contain a valid SQL expression.
     *  The SQL expression specified for a column is used to transform or compute values on the fly during insertion.
     *  The SQL expression can depend on columns that exist in the table.
     *  The SQL expression can also depend on values or columns that do not exist in the table, but must be specified in the `inserterDefinition`.
     * \param inserterDefinition The definition of columns to which values are provided.
     *  The column definition for all the columns without SQL expression must be specified in `inserterDefinition`.
     *  For a column without SQL expression the column definition provided in `inserterDefinition` must match the actual definition of the column in the table.
     *  All columns that SQL expressions specified in `columnMappings` must be in `inserterDefinition`.
     * \throws HyperException
     * \pre connection.isOpen()
     * \pre all columns in columnMappings are part of the table definition
     * \pre columns without expressions are provided in the inserter definition
     * \post isOpen()

     * Consider the following pseudo-code on how to transform or compute values on the fly during insertion:
     * TableDefinition    : [TableName="example", Columns=["ColumnA" as INT, "ColumnB" as BIG_INT]]
     * ColumnMapping      : [[Name: "ColumnA"], [Name:"ColumnB", Expression:"ColumnA"*"ColumnC"]]
     * InserterDefinition : ["ColumnA" integer, "ColumnC" integer]
     *
     * Notice that "ColumnA" does not specify an expression and "ColumnB" is a product of "ColumnA" and "ColumnC", but "ColumnC" is not part of the table.
     * The InserterDefinition contains "ColumnA" and "ColumnC"
     *   "ColumnA" since it is not computed on the fly and has to be provided to the inserter
     *   "ColumnC" since it is specified in the SQL expression that computes "ColumnB" on the fly
     *
     * try (Inserter inserter(conn, "example", columnMapping, inserterDefinition)) {
     *      inserter.add(2).add(3).endRow();
     *      inserter.execute();
     * }
     * The insertion code snippet above inserts 2 into "ColumnA" and 6 into "ColumnB" (product of 2 and 3)
     */
   Inserter(Connection& connection, const TableName& name, std::vector<Inserter::ColumnMapping> columnMappings, std::vector<TableDefinition::Column> inserterDefinition);

   /**
     * Constructs an `Inserter` object that does not represent an inserter.
     * \post !isOpen()
     * \throw bad_alloc
     */
   Inserter() : tableDefinition(TableName({})), streamDefinition(TableName({})) {}

   /**
     * Destroys the inserter. Discards all data if the insert was not executed.
     */
   ~Inserter() noexcept;

   /** Move constructor */
   Inserter(Inserter&& other) noexcept;

   /** Move assignment */
   Inserter& operator=(Inserter&& other) noexcept;

   /** Copy forbidden */
   Inserter(const Inserter&) = delete;
   Inserter& operator=(const Inserter&) = delete;

   /**
     * Sets the current field to the given value.
     *
     * If the current field is nullable, you can insert optional<Type>.
     *
     * The following types are supported:
     * short
     * int
     * long
     * long long
     * bool
     * double
     * hyperapi::Numeric<precision, scale>
     * uint32_t
     * hyperapi::string_view
     * ByteSpan
     * hyperapi::Interval
     * hyperapi::Date
     * hyperapi::Time
     * hyperapi::Timestamp
     * hyperapi::OffsetTimestamp
     *
     *
     * Calling the method advances the current field.
     * \tparam ValueType  The type of the value to insert.
     * \param value  The value.
     * \return `*this`, to allow call chaining
     * \pre isOpen()
     * \pre The SQL type of the current column has to be compatible.
     * \pre There is at least one column left in the current row.
     * \throws HyperException
     */
   template <class ValueType>
   Inserter& add(ValueType value);

   /**
     * Inserts all given values. Implicitly calls `endRow()`.
     * \see add(ValueType value)
     * \return `*this`, to allow call chaining
     * \pre isOpen()
     * \pre The SQL type of the current column has to be compatible.
     * \pre There is at least one column left in the current row.
     */
   template <class... ValueTypes>
   Inserter& addRow(ValueTypes... values);

   /**
     * Advances the inserter to the next row.
     *
     * May cause the inserter to send data over the connection.
     * The next `add` method will insert into the first column again.
     * \return `*this`, to allow call chaining.
     * \throws HyperException
     * \pre isOpen()
     * \pre All columns of the current row are added.
     */
   Inserter& endRow();

   /**
     * Returns whether the inserter is open.
     */
   bool isOpen() const noexcept;

   /**
     * Submits the previously added data.
     * If this operation succeeds without throwing, all previously added data is sent to Hyper;
     * otherwise all data is discarded.
     * In either way, the inserter will be closed once this method completes.
     * \throws HyperException
     * \pre isOpen()
     * \pre `endRow()` was called for all inserted rows.
     * \post !isOpen()
     */
   void execute();

   /**
     * Closes the inserter.
     *
     * Closing the inserter discards all data if the insert was not executed.
     * \post !isOpen()
     */
   void close() noexcept;

   private:
   /**
     * Initializes a new chunk.
     * \pre isOpen()
     */
   void newChunk() noexcept;

   /**
     * Sends the current chunk over the connection.
     * \pre isOpen()
     * \throws HyperException
     */
   void sendChunk();

   private:
   /** The table definition of the table into which this inserter inserts data. */
   hyperapi::TableDefinition tableDefinition;
   /** The table definition c-handle, has to be kept alive during insertion */
   internal::HyperTableDefinition tableDefinitionHandle;
   /** The table definition of the stream when calculated expressions or bulk insert is used. */
   hyperapi::TableDefinition streamDefinition;
   /** The stream definition c-handle, has to be kept alive during insertion */
   internal::HyperTableDefinition streamDefinitionHandle;
   /** The underlying handle. */
   hyper_inserter_t* inserter = nullptr;
   /** The current chunk. */
   std::vector<uint8_t> currentChunk;
   /** The offset to the next free byte in the current chunk. */
   size_t chunkOffset = 0;
   /** The size of the header in the current chunk. */
   size_t headerSize = 0;
   /** The index of the current field */
   hyper_field_index_t currentField = 0;
   /** The select list of the inserter */
   std::string selectList;

   friend class InserterTest;
   friend struct internal::ValueInserter;
};
}

#include <hyperapi/impl/Inserter.impl.hpp>

#endif
