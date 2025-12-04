#pragma once
#include <string>

namespace xlog {

enum class Color {
    None,
    Red,
    Yellow,
    Blue,
    Green
};

std::string apply_color(const std::string& text, Color color);

}
