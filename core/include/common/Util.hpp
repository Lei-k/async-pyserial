#ifndef COMMON_UTIL_HPP
#define COMMON_UTIL_HPP

#include <string>

namespace common {
  std::string wstring_to_string(const std::wstring& wstr) {
      if (wstr.empty()) {
          return std::string();
      }
      int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
      std::string str(size_needed, 0);
      WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
      return str;
  }
}

#endif