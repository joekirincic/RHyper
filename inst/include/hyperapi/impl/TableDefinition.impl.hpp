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

#ifndef TABLEAU_HYPER_IMPL_TABLEDEFINITION_IMPL_HPP
#define TABLEAU_HYPER_IMPL_TABLEDEFINITION_IMPL_HPP

#include <algorithm>
#include <hyperapi/HyperException.hpp>
#include <string>
#include <hyperapi/hyperapi.h>

namespace hyperapi {

namespace internal {
class HyperTableDefinition final {
   /// The underlying pointer
   hyper_table_definition_t* tableDefinition = nullptr;

   public:
   /// Constructor
   HyperTableDefinition() noexcept = default;
   /// Constructor
   HyperTableDefinition(const TableDefinition& t);
   /// Destructor
   ~HyperTableDefinition();

   /// Move constructor
   HyperTableDefinition(HyperTableDefinition&&);
   /// Move assignment
   HyperTableDefinition& operator=(HyperTableDefinition&&);

   /// Copy forbidden
   HyperTableDefinition(const HyperTableDefinition&) = delete;
   HyperTableDefinition& operator=(const HyperTableDefinition&) = delete;

   /**
     * Returns the underlying `hyper_table_definition_t*`.
     */
   hyper_table_definition_t* get() { return tableDefinition; }
};

inline HyperTableDefinition::HyperTableDefinition(const TableDefinition& tableDefinition) {
   const TableName& t = tableDefinition.getTableName();
   const char* tableName = t.getName().getUnescaped().c_str();
   const char* schemaName = t.getSchemaName() ? t.getSchemaName()->getName().getUnescaped().c_str() : "";
   const char* databaseName = t.getDatabaseName() ? t.getDatabaseName()->getName().getUnescaped().c_str() : "";

   hyper_table_persistence_t tablePersistence;
   switch (tableDefinition.getPersistence()) {
      case Persistence::Permanent:
         tablePersistence = hyper_table_persistence_t::HYPER_PERMANENT;
         break;
      case Persistence::Temporary:
         tablePersistence = hyper_table_persistence_t::HYPER_TEMPORARY;
         break;
   }

   this->tableDefinition = hyper_create_table_definition(databaseName, schemaName, tableName, tablePersistence, false);

   for (const TableDefinition::Column& c : tableDefinition.columns) {
      const SqlType& cType = c.getType();
      const char* collation = c.getCollation().empty() ? nullptr : c.getCollation().c_str();
      if (hyper_error_t* error = hyper_table_definition_add_column(this->tableDefinition, c.getName().getUnescaped().c_str(), static_cast<hyper_type_tag_t>(cType.tag), cType.modifier, collation, c.getNullability() == Nullability::Nullable)) {
         throw internal::makeHyperException(error);
      }
   }
}

inline HyperTableDefinition::~HyperTableDefinition() {
   if (tableDefinition) {
      hyper_destroy_table_definition(tableDefinition);
   }
}

inline HyperTableDefinition::HyperTableDefinition(HyperTableDefinition&& other)
   : tableDefinition(other.tableDefinition) {
   other.tableDefinition = nullptr;
}

inline HyperTableDefinition& HyperTableDefinition::operator=(HyperTableDefinition&& other) {
   if (this != &other) {
      if (tableDefinition) {
         hyper_destroy_table_definition(tableDefinition);
      }
      tableDefinition = other.tableDefinition;
      other.tableDefinition = nullptr;
   }
   return *this;
}

template <class T>
struct NullabilityGetter {
   Nullability get() noexcept { return Nullability::NotNullable; }
};

template <class T>
struct NullabilityGetter<optional<T>> {
   inline Nullability get() noexcept { return Nullability::Nullable; }
};
template <class T>
inline Nullability getNullability() noexcept {
   return NullabilityGetter<T>().get();
}
}

inline std::ostream& operator<<(std::ostream& os, Nullability nullability) {
   if (nullability == Nullability::Nullable) {
      os << "Nullable";
   } else if (nullability == Nullability::NotNullable) {
      os << "NotNullable";
   }
   return os;
}

inline TableDefinition::Column::Column(Name name, SqlType type, Nullability nullability)
   : name(std::move(name)), type(type), nullability(nullability) {
}

inline TableDefinition::Column::Column(
   Name name, SqlType type, std::string collation, Nullability nullability)
   : name(std::move(name)), type(type), nullability(nullability), collation(std::move(collation)) {
}

inline TableDefinition::TableDefinition(TableName name, Persistence persistence)
   : name(std::move(name)), persistence(persistence) {
}

inline TableDefinition::TableDefinition(TableName name, std::vector<Column> columns, Persistence persistence)
   : name(std::move(name)), persistence(persistence), columns(std::move(columns)) {
}

inline const TableDefinition::Column& TableDefinition::getColumn(hyper_field_index_t columnIndex) const {
   HYPER_PRECONDITION(columnIndex < getColumnCount());
   return columns[columnIndex];
}

inline const TableDefinition::Column* TableDefinition::getColumnByName(const Name& s) const noexcept {
   optional<hyper_field_index_t> columnIndex = getColumnPositionByName(s);
   return (!columnIndex.has_value()) ? nullptr : &columns[*columnIndex];
}

inline optional<hyper_field_index_t> TableDefinition::getColumnPositionByName(const Name& s) const noexcept {
   for (hyper_field_index_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
      if (columns[columnIndex].getName() == s) {
         return columnIndex;
      }
   }
   return {};
}

inline TableDefinition& TableDefinition::addColumn(Column&& c) noexcept {
   columns.push_back(std::move(c));
   return *this;
}

inline TableDefinition& TableDefinition::addColumn(const Column& c) noexcept {
   columns.emplace_back(c);
   return *this;
}

inline TableDefinition& TableDefinition::setTableName(TableName n) noexcept {
   name = std::move(n);
   return *this;
}

inline TableDefinition& TableDefinition::setPersistence(Persistence p) noexcept {
   persistence = p;
   return *this;
}
}
#endif
