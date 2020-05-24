#pragma once

#include <unordered_map>
#include "ast/ast_internal.hpp"
#include "interpreter_internal.hpp"

struct expression_and_constraint
{
    expression_shared_ptr expression;
    constraint_const_shared_ptr constraint;
};

using expression_and_constraint_shared = std::shared_ptr<expression_and_constraint>;

struct compilation_cache
{
    struct frame
    {
    friend struct compilation_cache;
    public:
        ~frame() { m_parent.pop_frame(); }
    protected:
        frame(compilation_cache& parent, const element_scope* function_scope) : m_parent(parent) { m_parent.push_frame(function_scope); }
        compilation_cache& m_parent;
    };

    compilation_cache() { m_cache.resize(1); } //todo: do we still need this?

    void add(const element_scope* scope, expression_and_constraint_shared expr) { m_cache.back().cache[scope] = std::move(expr); }

    frame add_frame(const element_scope* function_scope) { return frame(*this, function_scope); }

    expression_and_constraint_shared search(const element_scope* scope) const
    {
        for (auto it = m_cache.rbegin(); it != m_cache.rend(); ++it) {
            auto mit = it->cache.find(scope);
            if (mit != it->cache.end())
                return mit->second;
        }
        return nullptr;
    }

    bool is_callstack_recursive(const element_scope* function_scope)
    {
        for (auto it = m_cache.rbegin(); it != m_cache.rend(); ++it) {
            if (it->function == function_scope)
                return true;
        }

        return false;
    }

private:
    using Cache = std::unordered_map<const element_scope*, expression_and_constraint_shared>;
    struct CacheEntry
    {
        Cache cache;
        const element_scope* function;
    };
    std::vector<CacheEntry> m_cache;

    void push_frame(const element_scope* function_scope) { m_cache.emplace_back(CacheEntry{ Cache{}, function_scope }); }
    void pop_frame() { m_cache.pop_back(); }
};


struct element_compiler_ctx
{
    element_interpreter_ctx& ictx;
    element_compiler_options options;
    compilation_cache comp_cache;
};

element_result element_compile(
    element_interpreter_ctx& ctx,
    const element_function* fn,
    expression_shared_ptr& expr,
    constraint_const_shared_ptr& constraint,
    element_compiler_options opts);
