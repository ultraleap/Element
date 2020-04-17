#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cstdlib>

#include "element/interpreter.h"
#include "ast_internal.hpp"
#include "ast/fwd.hpp"


struct element_scope
{
    const element_scope* parent = nullptr;
    std::string name;
    const element_ast* node = nullptr;
    std::unordered_map<std::string, scope_unique_ptr> children;

    element_scope(const element_scope* p, std::string n, const element_ast* nd)
        : parent(p), name(std::move(n)), node(nd)
    {
    }

    element_scope(const element_scope* p, std::string n, const element_ast* nd, function_const_shared_ptr fn)
        : element_scope(p, n, nd)
    {
        m_function = std::move(fn);
        m_function_cached = true;
    }

    virtual ~element_scope() {}

    const element_scope* lookup(std::string search, bool recurse = true) const;
    const element_scope* lookup(const std::vector<std::string>& search, bool recurse = true) const;

    element_item_type item_type() const;
    std::string qualified_name() const;

    function_const_shared_ptr function() const;

    const element_scope* root() const { return (parent ? parent->root() : this); }

private:
    const element_scope* lookup(const std::vector<std::string>& search, size_t idx, bool recurse) const;

    mutable function_const_shared_ptr m_function;
    mutable bool m_function_cached = false;
};

static inline scope_unique_ptr scope_new(const element_scope* parent, std::string name, const element_ast* node)
{
    return std::make_unique<element_scope>(parent, std::move(name), node);
}

static inline scope_unique_ptr scope_new_anonymous(const element_scope* parent, const element_ast* node)
{
    auto s = scope_new(parent, "", node);
    // create a locally-unique identifier
    s->name = std::string("#anonymous_") + std::to_string(uintptr_t(parent) ^ uintptr_t(node));
    return std::move(s);
}