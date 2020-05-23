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

struct expression_cache
{
    struct frame
    {
    friend struct expression_cache;
    public:
        ~frame() { m_parent.pop_frame(); }
    protected:
        frame(expression_cache& parent) : m_parent(parent) { m_parent.push_frame(); }
        expression_cache& m_parent;
    };

    expression_cache() { m_cache.resize(1); }

    void add(const element_scope* scope, expression_and_constraint_shared expr) { m_cache.back()[scope] = expr; }

    frame add_frame() { return frame(*this); }

    expression_and_constraint_shared search(const element_scope* scope) const
    {
        for (auto it = m_cache.rbegin(); it != m_cache.rend(); ++it) {
            auto mit = it->find(scope);
            if (mit != it->end())
                return mit->second;
        }
        return nullptr;
    }

private:
    std::vector<std::unordered_map<const element_scope*, expression_and_constraint_shared>> m_cache;

    void push_frame() { m_cache.emplace_back(); }
    void pop_frame() { m_cache.pop_back(); }
};


struct element_compiler_ctx
{
    element_interpreter_ctx& ictx;
    element_compiler_options options;
    expression_cache expr_cache;
};

element_result element_compile(
    element_interpreter_ctx& ctx,
    const element_function* fn,
    expression_shared_ptr& expr,
    element_compiler_options opts);
