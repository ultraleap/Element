#include "port.hpp"

namespace element
{
    port::port(identifier name, std::unique_ptr<type_annotation> annotation)
        : name{ std::move(name) }
        , annotation{ std::move(annotation) }
    {
    }
}
