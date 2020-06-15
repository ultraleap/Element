#include "port.hpp"

element::port::port(element::identifier identifier, std::unique_ptr<type_annotation> annotation)
    : identifier{std::move(identifier)}, annotation{std::move(annotation)}
{
}
