#pragma once
#include <string>

namespace Zyrnix {

enum class Color {
    None,
    Red,
    Yellow,
    Blue,
    Green
};

std::string apply_color(const std::string& text, Color color);

}
