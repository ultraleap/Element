#include "declaration.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "../intermediaries/declaration_wrapper.hpp"
#include "../scope.hpp"

using namespace element;

declaration::declaration(identifier name, const scope* parent)
    : name(std::move(name))
    , our_scope(std::make_unique<scope>(parent, this))
    , wrapper(std::make_shared<declaration_wrapper>(this))
{
    assert(parent);
}

bool declaration::has_scope() const
{
    return !our_scope->is_empty();
}

std::string declaration::location() const
{
    return name.value;
    //assert(our_scope && our_scope->get_parent_scope());

    //if (our_scope->get_parent_scope()->is_root())
    //    return name.value;

    ////recursive construction
    //return fmt::format("{}.{}", our_scope->get_parent_scope()->location(), name.value);
}