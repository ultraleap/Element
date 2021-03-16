#include "scope_caches.hpp"

//SELF
#include "scope.hpp"

//STD
#include <exception>

using namespace element;

void scope_caches::clear()
{
    marked_for_clearing = false;
    cache.clear();
}

void scope_caches::mark_to_clear()
{
    marked_for_clearing = true;
}

scope_caches::find_map& scope_caches::get(const scope* scope)
{
    if (marked_for_clearing)
        clear();

    return cache[scope];
}
