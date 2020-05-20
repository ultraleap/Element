#include "ast/scope.hpp"

#include <functional>
#include <cassert>
#include "common_internal.hpp"
#include "interpreter_internal.hpp"
#include "ast/functions.hpp"
#include "ast/ast_indexes.hpp"
#include "stringutil.hpp"

//todo: optimise all of this to use string_view
const element_scope* element_scope::lookup(std::string search, bool recurse) const
{
    return lookup(split<'.'>(search), 0, recurse);
}

const element_scope* element_scope::lookup(const std::vector<std::string>& search, bool recurse) const
{
    return lookup(search, 0, recurse);
}

const element_scope* element_scope::lookup(const std::vector<std::string>& search, size_t idx, bool recurse) const
{
    // see if we have a child by this name
    const auto name_it = children.find(search[idx]);
    if (name_it != children.end()) {
        // are we at the end of our search?
        if (idx + 1 >= search.size())
            return name_it->second.get();
        else
            return name_it->second->lookup(search, idx + 1, false);
    }
    // search parent if appropriate
    return (recurse && parent) ? parent->lookup(search, idx, true) : nullptr;
}

std::string element_scope::qualified_name() const
{
    if (parent && parent->item_type() != ELEMENT_AST_NODE_ROOT)
        return parent->qualified_name() + std::string(".") + name;
    else
        return name;
}

element_item_type element_scope::item_type() const
{
    switch (node->type) {
    case ELEMENT_AST_NODE_ROOT:
        return ELEMENT_ITEM_ROOT;
    case ELEMENT_AST_NODE_FUNCTION:
        assert(node->children.size() > ast_idx::fn::body);
        return (node->children[ast_idx::fn::body]->type == ELEMENT_AST_NODE_CONSTRAINT)
            ? ELEMENT_ITEM_CONSTRAINT
            : ELEMENT_ITEM_FUNCTION;
    case ELEMENT_AST_NODE_STRUCT:
        return ELEMENT_ITEM_STRUCT;
    case ELEMENT_AST_NODE_NAMESPACE:
        return ELEMENT_ITEM_NAMESPACE;
    default:
        assert(false);
        return ELEMENT_ITEM_UNKNOWN;
    }
}

function_const_shared_ptr element_scope::function() const
{
    // Return if already generated
    if (m_function_cached)
        return m_function;
    // Check if it makes sense for the node we have
    // TODO: this needs to change
    if (node->children.size() > ast_idx::fn::declaration) {
        auto decl = node->children[ast_idx::fn::declaration].get();
        if (decl->type == ELEMENT_AST_NODE_DECLARATION && decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC)) {
            // Check name against builtins
            m_function = element_function::get_builtin(node->children[0]->identifier);
            if (!m_function) {
                // Didn't find intrinsic; was a fallback Element version provided?
            }
        }
    }

    if (!m_function) {
        if (node->type == ELEMENT_AST_NODE_FUNCTION) {
            m_function = std::make_shared<element_custom_function>(this);
        } else if (node->type == ELEMENT_AST_NODE_STRUCT) {
            auto type = std::make_shared<element_custom_type>(this);
            m_function = std::make_shared<element_type_ctor>(type);
        }
    }

    m_function_cached = true;
    return m_function;
}