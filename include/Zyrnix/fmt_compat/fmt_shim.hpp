#pragma once
#include <fmt/core.h>
#include <fmt/format.h>

namespace Zyrnix {

template<typename... Args>
inline std::string format(const std::string& fmt_str, Args&&... args) {
    return fmt::format(fmt_str, std::forward<Args>(args)...);
}

}
