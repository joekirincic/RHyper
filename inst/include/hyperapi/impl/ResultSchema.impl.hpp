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

#ifndef TABLEAU_HYPER_IMPL_RESULTSCHEMA_IMPL_HPP
#define TABLEAU_HYPER_IMPL_RESULTSCHEMA_IMPL_HPP

#include <hyperapi/impl/infra.hpp>

namespace hyperapi {

inline ResultSchema::Column::Column(Name&& name, SqlType&& type)
   : name(std::move(name)), type(std::move(type)) {
}

inline const ResultSchema::Column& ResultSchema::getColumn(hyper_field_index_t columnIndex) const {
   HYPER_PRECONDITION(static_cast<size_t>(columnIndex) < getColumnCount());
   return columns[static_cast<size_t>(columnIndex)];
}

inline const ResultSchema::Column* ResultSchema::getColumnByName(const Name& name) const {
   optional<hyper_field_index_t> columnIndex = getColumnPositionByName(name);
   return (!columnIndex.has_value()) ? nullptr : &getColumn(*columnIndex);
}

inline optional<hyper_field_index_t> ResultSchema::getColumnPositionByName(const Name& name) const {
   for (hyper_field_index_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
      if (columns[columnIndex].getName() == name) {
         return columnIndex;
      }
   }
   return {};
}
}

#endif
