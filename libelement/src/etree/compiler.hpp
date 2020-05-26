#pragma once

#include <unordered_map>
#include "ast/ast_internal.hpp"
#include "interpreter_internal.hpp"

struct compilation
{
    expression_shared_ptr expression;
    constraint_const_shared_ptr constraint;
};

//todo: consider the implications of using and not using it as shared_ptr, given that it's already containing shared_ptrs
using compilation_shared_ptr = std::shared_ptr<compilation>;

struct compilation_cache
{
    struct frame
    {
    friend struct compilation_cache;
    public:
        //todo: enable these as needed, with correct implementations  
        frame(const frame& other) = delete;
        frame(frame&& other) = delete;
        frame& operator=(const frame& other) = delete;
        frame& operator=(frame&& other) = delete;

        ~frame() { m_parent.pop_frame(); }
    protected:
        frame(compilation_cache& parent, const element_scope* function_scope) : m_parent(parent) { m_parent.push_frame(function_scope); }
        compilation_cache& m_parent;
    };

    //todo: is this meant to be some kind of global cache entry? we should pass the global scope to it in that case. I'm not sure we're using it anyway right now
    compilation_cache() { m_cache.resize(1); } //todo: do we still need this?

    //todo: we should probably not allow reassignment of existing function entries
    void add(const element_scope* scope, compilation_shared_ptr expr) { m_cache.back().cache[scope] = std::move(expr); }

    frame add_frame(const element_scope* function_scope) { return frame(*this, function_scope); }

    compilation_shared_ptr search(const element_scope* scope) const
    {
        for (auto it = m_cache.rbegin(); it != m_cache.rend(); ++it) {
            auto mit = it->cache.find(scope);
            if (mit != it->cache.end())
                return mit->second;
        }
        return nullptr;
    }

    //todo: this only makes sense to have here if all the scopes are functions that we've called, so maybe I haven't thought about this enough and it should be its own class
    bool is_callstack_recursive(const element_scope* function_scope)
    {
        for (auto it = m_cache.rbegin(); it != m_cache.rend(); ++it) {
            if (it->function == function_scope)
                return true;
        }

        return false;
    }

private:
    using Cache = std::unordered_map<const element_scope*, compilation_shared_ptr>;
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
