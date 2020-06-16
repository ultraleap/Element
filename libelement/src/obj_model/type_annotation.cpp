#include "type_annotation.hpp"

std::string element::type_annotation::to_code(const int depth) const
{
    return ":" + name.value;
}