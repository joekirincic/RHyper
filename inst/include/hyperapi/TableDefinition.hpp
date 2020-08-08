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

#ifndef TABLEAU_HYPER_TABLEDEFINITION_HPP
#define TABLEAU_HYPER_TABLEDEFINITION_HPP

#include <hyperapi/HyperException.hpp>
#include <hyperapi/SqlType.hpp>
#include <hyperapi/TableName.hpp>
#include <hyperapi/impl/infra.hpp>
#include <hyperapi/optional.hpp>
#include <ostream>
#include <string>
#include <vector>
#include <hyperapi/hyperapi.h>

namespace hyperapi {

/**
  * The nullability of a column. Specifies whether the column can contain NULL values.
  */
enum Nullability : bool {
   /** The column can contain NULL values */
   Nullable = true,
   /** The column cannot contain NULL values */
   NotNullable = false
};

/** Stream output operator for `Nullability` */
std::ostream& operator<<(std::ostream& os, Nullability nullability);

/** Possible persistence levels for database objects */
enum class Persistence {
   /** Permanent */
   Permanent,
   /** Temporary: Only available in the own session, not persisted */
   Temporary
};

/**
  * A table definition. Contains a table name as well as possibly multiple descriptions of columns.
  */
class TableDefinition final {
   public:
   /** Type of a column index. */
   using column_index_type = hyper_field_index_t;

   /** A Column of a table definition */
   class Column final {
      public:
      /**
        * Returns the name of the column.
        */
      const Name& getName() const noexcept { return name; }

      /**
        * Returns the type of the column.
        */
      const SqlType& getType() const noexcept { return type; }

      /**
        * Returns the Nullability of the column.
        */
      Nullability getNullability() const noexcept { return nullability; }

      /**
        * Returns the collation of the column.
        */
      const std::string& getCollation() const noexcept { return collation; }

      /**
        * Creates a column.
        *
        * \param name  The name
        * \param type  The type
        * \param nullability The nullability
        */
      Column(Name name, SqlType type, Nullability nullability = Nullability::Nullable);

      /**
        * Creates a column.
        *
        * \param name  The name
        * \param type  The type
        * \param collation  The collation
        * \param nullability  The nullability
        * \pre The type supports collations.
        */
      Column(Name name, SqlType type, std::string collation, Nullability nullability = Nullability::Nullable);

      private:
      /// The name
      const Name name;
      /// The type
      const SqlType type;
      /// The Nullability
      const Nullability nullability;
      /// The collation
      const std::string collation;
   };

   public:
   /**
     * Creates a table definition with the given name and no columns.
     *
     * \param name  The name of the table.
     * \param persistence  The persistence of the table.
     */
   explicit TableDefinition(TableName name, Persistence persistence = Persistence::Permanent);

   /**
     * Creates a table definition with the given name and columns.
     *
     * \param name  The name of the table.
     * \param columns  The columns of the table.
     * \param persistence  The persistence of the table.
     */
   explicit TableDefinition(TableName name, std::vector<Column> columns, Persistence persistence = Persistence::Permanent);

   /**
     * Returns all columns.
     */
   const std::vector<Column>& getColumns() const noexcept { return columns; }

   /**
     * Returns the column at the given position.
     * \param columnIndex  The index of the column
     * \return The column at the given position (staring with 0).
     * \pre 0 <= columnIndex < getColumnCount()
     */
   const Column& getColumn(hyper_field_index_t columnIndex) const;

   /**
     * Returns the column with the given name.
     * \param s The name of the column
     * \return The column with the given name or `nullptr` if no such column exists.
     */
   const Column* getColumnByName(const Name& s) const noexcept;

   /**
     * Returns the position of the column with the given name.
     * \param s  The name of the column
     * \return The position of the column with the given name if such column exists.
     */
   optional<hyper_field_index_t> getColumnPositionByName(const Name& s) const noexcept;

   /**
     * Returns the numbers of columns.
     */
   size_t getColumnCount() const noexcept { return columns.size(); }

   /**
     * Returns the table persistence.
     */
   Persistence getPersistence() const noexcept { return persistence; }

   /**
     * Returns the name of the table.
     */
   const TableName& getTableName() const noexcept { return name; }

   /**
     * Adds a column to the definition.
     * \param c  The column to add.
     * \return `*this`, to allow call chaining.
     */
   TableDefinition& addColumn(Column&& c) noexcept;

   /** \copydoc addColumn(Column&& c) */
   TableDefinition& addColumn(const Column& c) noexcept;

   /**
     * Sets the table's name.
     * \param n The new name.
     * \return `*this`, to allow call chaining.
     */
   TableDefinition& setTableName(TableName n) noexcept;

   /**
     * Sets the table's persistence.
     * \param p The new persistence
     * \return `*this`, to allow call chaining.
     */
   TableDefinition& setPersistence(Persistence p) noexcept;

   private:
   /// The name of the table
   TableName name;
   /// The persistence
   Persistence persistence;
   /// The columns
   std::vector<Column> columns;

   friend class internal::HyperTableDefinition;
};
}

#include <hyperapi/impl/TableDefinition.impl.hpp>

#endif
