#include <common/util.h>

using namespace async_pyserial;

std::string common::wstring_to_string(const std::wstring &wstr)
{
    if (wstr.empty()) {
        return std::string();
    }
    
    // Create a converter object
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    
    // Convert wstring to string
    return converter.to_bytes(wstr);
}