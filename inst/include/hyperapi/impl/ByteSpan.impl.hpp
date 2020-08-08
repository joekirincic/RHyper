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

#ifndef TABLEAU_HYPER_IMPL_BYTESPAN_IMPL_HPP
#define TABLEAU_HYPER_IMPL_BYTESPAN_IMPL_HPP

#include <cctype>
#include <iomanip>

namespace hyperapi {

inline std::string ByteSpan::toString() const {
   std::ostringstream ss;
   for (size_t i = 0; i < size; ++i) {
      if (std::isprint(data[i])) {
         ss << data[i];
      } else {
         ss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
      }
   }
   return ss.str();
}

inline bool operator==(const ByteSpan& lhs, const ByteSpan& rhs) noexcept {
   if (lhs.size != rhs.size) {
      return false;
   }
   return (std::memcmp(lhs.data, rhs.data, lhs.size) == 0);
}

inline bool operator>(const ByteSpan& lhs, const ByteSpan& rhs) noexcept {
   int cmp = std::memcmp(lhs.data, rhs.data, lhs.size);
   if (cmp != 0) {
      return (cmp > 0);
   }
   return (lhs.size > rhs.size);
}
}

#endif
