#pragma once

//STD
#include <unordered_map>
#include <string>
#include <functional>

namespace element
{
class scope;
class declaration;

class scope_caches
{
public:
    using find_map = std::unordered_map<std::string, const declaration*>;
    using scope_map = std::unordered_map<const scope*, find_map>;

    void mark_to_clear();
    find_map& get(const scope* scope);

private:
    void clear();

    bool marked_for_clearing = false;
    //todo: 3rd party hashmaps like robinhood or absl could be up to twice as fast if this is a big bottleneck
    scope_map cache;
};
} // namespace element