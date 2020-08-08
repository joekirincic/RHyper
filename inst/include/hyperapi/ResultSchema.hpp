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

#ifndef TABLEAU_HYPER_RESULTSCHEMA_HPP
#define TABLEAU_HYPER_RESULTSCHEMA_HPP

#include <hyperapi/Name.hpp>
#include <hyperapi/SqlType.hpp>
#include <hyperapi/optional.hpp>
#include <vector>

namespace hyperapi {

class Result;

/**
 * A result schema.
 */
class ResultSchema final {
   public:
   /** A column of a result. */
   class Column final {
      friend class ResultSchemaTest;
      friend class Result;

      public:
      /** Gets the column name */
      const Name& getName() const noexcept { return name; }
      /** Gets the type */
      const SqlType& getType() const noexcept { return type; }

      private:
      /** The name */
      const Name name;
      /** The type */
      const SqlType type;

      /** Constructor */
      Column(Name&& name, SqlType&& type);
   };

   public:
   /**
     * Returns all columns.
     */
   const std::vector<Column>& getColumns() const noexcept { return columns; }

   /**
     * Returns the column at the given index.
     *
     * \param columnIndex  The index of the column.
     * \return The column.
     * \pre 0 <= columnIndex < getColumnCount()
     */
   const Column& getColumn(hyper_field_index_t columnIndex) const;

   /**
     * Returns the number of columns.
     */
   size_t getColumnCount() const noexcept { return columns.size(); }

   /**
     * Gets the column with the given name.
     *
     * \param name  The name of the column
     * \return A pointer to the column, or `nullptr` if such a column does not exist.
     */
   const Column* getColumnByName(const Name& name) const;

   /**
     * Gets the position of the column with the given name.
     *
     * \param name  The name of the column.
     * \return The position of the column in the result schema if it exists.
     */
   optional<hyper_field_index_t> getColumnPositionByName(const Name& name) const;

   private:
   // TODO 940803 Avoid private methods; move into impl header
   /**
     * Adds a column to the result schema.
     * \param column  The column to add.
     */
   void addColumn(Column&& column) { columns.emplace_back(std::move(column)); }

   /// The columns
   std::vector<Column> columns;

   friend class ResultSchemaTest;
   friend class Result;
};
}

#include <hyperapi/impl/ResultSchema.impl.hpp>

#endif
