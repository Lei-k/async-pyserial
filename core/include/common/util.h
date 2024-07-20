#ifdef Win32

#ifndef ASYNC_PYSERIAL_WIN32_UTIL_H
#define ASYNC_PYSERIAL_WIN32_UTIL_H

#include <string>
#include <locale>
#include <codecvt>


namespace async_pyserial
{
    namespace common
    {
        std::string wstring_to_string(const std::wstring &wstr)
        {
            if (wstr.empty()) {
                return std::string();
            }
            
            // Create a converter object
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            
            // Convert wstring to string
            return converter.to_bytes(wstr);
        }
    }
}

#endif

#endif