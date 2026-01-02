#pragma once
#include "fmt_shim.hpp"
#include <string>

namespace Zyrnix {

template<typename... Args>
inline std::string fmt_format(const std::string& fmt_str, Args&&... args) {
    return format(fmt_str, std::forward<Args>(args)...);
}

}
