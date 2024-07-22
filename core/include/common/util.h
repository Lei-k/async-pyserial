#ifndef ASYNC_PYSERIAL_COMMON_UTIL_H
#define ASYNC_PYSERIAL_COMMON_UTIL_H

#include <string>
#include <locale>
#include <codecvt>


namespace async_pyserial
{
    namespace common
    {
        std::string wstring_to_string(const std::wstring &wstr);
    }
}

#endif