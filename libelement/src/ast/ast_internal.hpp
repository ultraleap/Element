#pragma once

#include "element/ast.h"

#include <vector>
#include <string>
#include <memory>
#include <functional>

using ast_unique_ptr = std::unique_ptr<element_ast, void(*)(element_ast*)>;

struct element_ast
{

    element_ast_node_type type;
    std::string identifier;
    union {
        element_value literal = 0;   // active for AST_NODE_LITERAL
        element_ast_flags flags; // active for all other node types
    };
    element_ast* parent = nullptr;
    std::vector<ast_unique_ptr> children;
    // TODO: track source token?

    bool has_flag(element_ast_flags flag) const 
    {
        return (flags & flag) == flag;
    }

    element_ast* find_child(std::function<bool(const element_ast* elem)> fn)
    {
        for (const auto& t : children) {
            if (fn(t.get()))
                return t.get();
        }
        return nullptr;
    }

    element_ast* first_child_of_type(element_ast_node_type type) const
    {
        for (const auto& t : children) {
            if (t->type == type)
                return t.get();
        }
        return nullptr;
    }

    template <size_t N>
    element_ast* nth_parent()
    {
        element_ast* p = this;
        for (size_t i = 0; i < N; ++i) {
            if (!p) break;
            p = p->parent;
        }
        return p;
    }

    template <size_t N>
    const element_ast* nth_parent() const
    {
        const element_ast* p = this;
        for (size_t i = 0; i < N; ++i) {
            if (!p) break;
            p = p->parent;
        }
        return p;
    }


    enum class walk_step { stop, step_in, next, step_out };
    using walker = std::function<walk_step(element_ast*)>;
    using const_walker = std::function<walk_step(const element_ast*)>;

    walk_step walk(const walker& fn);
    walk_step walk(const const_walker& fn) const;

    element_ast(element_ast* iparent) : parent(iparent) {}
};


inline bool ast_node_has_identifier(const element_ast* n)
{
    return n->type == ELEMENT_AST_NODE_DECLARATION
        || n->type == ELEMENT_AST_NODE_IDENTIFIER
        || n->type == ELEMENT_AST_NODE_CALL
        || n->type == ELEMENT_AST_NODE_PORT;
}

inline bool ast_node_has_literal(const element_ast* n)
{
    return n->type == ELEMENT_AST_NODE_LITERAL;
}
