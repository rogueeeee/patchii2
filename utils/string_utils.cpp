#include "string_utils.h"
#include <Windows.h>

std::wstring random_wstring(std::size_t length, std::wstring_view set)
{
    static auto setup_srand = []() -> auto
    {
        #pragma warning (disable: 28159)
        std::srand(GetTickCount());
        #pragma warning (default: 28159)
        return 0;
    }();

    std::wstring result;

    for (std::size_t idx = 0; idx < length; idx++)
        result += set[std::rand() % set.length()];

    return result;
}
