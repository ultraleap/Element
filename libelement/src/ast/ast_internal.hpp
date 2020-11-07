#pragma once

//STD
#include <vector>
#include <string>
#include <memory>
#include <functional>

//SELF
#include "ast_indexes.hpp"
#include "common_internal.hpp"
#include "element/ast.h"
#include "element/token.h"
#include <cassert>

using ast_unique_ptr = std::unique_ptr<element_ast, void (*)(element_ast*)>;

struct element_ast
{
public:
    element_ast(element_ast* ast_parent);

    static void move(element_ast* from, element_ast* to, bool reparent);

    element_ast* new_child(element_ast_node_type type = ELEMENT_AST_NODE_NONE);
    void clear_children();

    [[nodiscard]] bool has_flag(element_ast_flags flag) const;
    [[nodiscard]] bool has_identifier() const;
    [[nodiscard]] bool has_literal() const;

    [[nodiscard]] bool in_function_scope() const;
    [[nodiscard]] bool in_lambda_scope() const;

    [[nodiscard]] bool struct_is_valid() const;
    [[nodiscard]] bool struct_has_body() const;
    [[nodiscard]] const element_ast* struct_get_declaration() const;
    [[nodiscard]] const element_ast* struct_get_body() const;

    [[nodiscard]] bool function_is_valid() const;
    [[nodiscard]] bool function_has_body() const;
    [[nodiscard]] const element_ast* function_get_declaration() const;
    [[nodiscard]] const element_ast* function_get_body() const;

    [[nodiscard]] bool declaration_is_valid() const;
    [[nodiscard]] bool declaration_has_inputs() const;
    [[nodiscard]] const element_ast* declaration_get_inputs() const;
    [[nodiscard]] bool declaration_has_outputs() const;
    [[nodiscard]] const element_ast* declaration_get_outputs() const;
    [[nodiscard]] bool declaration_is_intrinsic() const;
    [[nodiscard]] bool declaration_has_portlist() const;

    [[nodiscard]] element_ast* get_root();
    [[nodiscard]] const element_ast* get_root() const;

    union
    {
        element_value literal = 0; // active for AST_NODE_LITERAL
        element_ast_flags flags;   // active for all other node types
    };

    element_ast_node_type type;
    std::string identifier;
    element_ast* parent = nullptr;
    std::vector<ast_unique_ptr> children;
    const element_token* nearest_token = nullptr;
};
