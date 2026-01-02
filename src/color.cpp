#include "Zyrnix/color.hpp"

namespace Zyrnix {

std::string apply_color(const std::string& text, Color color) {
    switch (color) {
        case Color::Red: return "\033[31m" + text + "\033[0m";
        case Color::Yellow: return "\033[33m" + text + "\033[0m";
        case Color::Blue: return "\033[34m" + text + "\033[0m";
        case Color::Green: return "\033[32m" + text + "\033[0m";
        default: return text;
    }
}

}
