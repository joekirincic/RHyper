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

#ifndef TABLEAU_HYPER_IMPL_STRING_VIEW_IMPL_HPP
#define TABLEAU_HYPER_IMPL_STRING_VIEW_IMPL_HPP

#include <algorithm>

namespace hyperapi {
inline int string_view::compare(const string_view& other) const noexcept {
   if (const int result = std::strncmp(data(), other.data(), (std::min)(size(), other.size()))) {
      return result;
   }

   return (size() == other.size() ? 0 : (size() < other.size() ? -1 : 1));
}
}

namespace std {
inline size_t hash<hyperapi::string_view>::operator()(const hyperapi::string_view& sv) const noexcept {
   size_t hash = 0;
   const char* svData = sv.data();
   for (size_t index = 0; index < sv.size(); ++index) {
      hash = 5 * hash + static_cast<size_t>(svData[index]);
   }
   return size_t(hash);
}
}
#endif
