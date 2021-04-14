#pragma once

#include <unordered_map>

#include "instructions.hpp"

namespace element
{
    struct instruction_cache_value
    {
        element_value value;
        bool present;
    };

    class instruction_cache
    {
    public:
        instruction_cache()
            : cache()
        {}

        explicit instruction_cache(const instruction* instruction)
            : cache()
        {
            initialise(instruction);
        }

        void clear_values()
        {
            for (auto& [key, val] : cache)
            {
                val.present = false;
            }
        }

        /**
         * Get a pointer to the value associated with the instruction in the cache
         * If no cache entry exists with that instruction, return nullptr
         */
        instruction_cache_value* find(const instruction* instruction)
        {
            const auto found = cache.find(instruction);
            if (found != cache.end())
            {
                return &(found->second);
            }
            return nullptr;
        }

    private:
        std::unordered_map<const instruction*, instruction_cache_value> cache;

        void initialise(const instruction* instruction)
        {
            cache.emplace(instruction, instruction_cache_value{ 0, false });
            for (const auto& dep : instruction->dependents())
            {
                initialise(dep.get());
            }
        }
    };

} // namespace element
