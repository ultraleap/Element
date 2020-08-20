#include <catch2/catch.hpp>
#include <iostream>

#include "element/ast.h"
#include "element/token.h"
#include "../src/ast/ast_internal.hpp"

//STD
#include <array>

void print_ast(element_ast* ast, int depth, element_ast* ast_to_mark)
{
    std::array<char, 2048> output_buffer{};
    element_ast_to_string(ast, depth, ast_to_mark, output_buffer.data(), output_buffer.size());
    printf("%s", output_buffer.data());
    UNSCOPED_INFO(output_buffer.data());
}

TEST_CASE("Ast Generation", "[AST]") {

    element_tokeniser_ctx* tokeniser;
    element_tokeniser_create(&tokeniser);

    element_parser_ctx parser;

    SECTION("Example Code - Simple Variable") {
        element_tokeniser_run(tokeniser, "Burger1 = 2;", "<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        // Root
        REQUIRE(parser.root->type == ELEMENT_AST_NODE_ROOT);

        auto root = parser.root;
        print_ast(root, 0, nullptr);

        // Function
        REQUIRE(root->children[0]->type == ELEMENT_AST_NODE_FUNCTION);

        // Declaration
        REQUIRE(root->children[0]->children[0]->type == ELEMENT_AST_NODE_DECLARATION);
        REQUIRE(root->children[0]->children[0]->identifier == "Burger1");

        // Empty inputs
        REQUIRE(root->children[0]->children[0]->children[0]->type == ELEMENT_AST_NODE_NONE);
        REQUIRE(root->children[0]->children[0]->children[0]->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT));

        // Implicit return
        REQUIRE(root->children[0]->children[0]->children[1]->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
        REQUIRE(root->children[0]->children[0]->children[1]->parent->type == ELEMENT_AST_NODE_DECLARATION);

        // Literal
        REQUIRE(root->children[0]->children[1]->type == ELEMENT_AST_NODE_LITERAL);
        REQUIRE(root->children[0]->children[1]->literal == 2);
    }

    SECTION("Example Code - Struct") {
        element_tokeniser_run(tokeniser,"struct MyStruct(a, b);" ,"<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        // Root
        REQUIRE(parser.root->type == ELEMENT_AST_NODE_ROOT);

        auto root = parser.root;
        print_ast(root, 0, nullptr);

        // Struct
        REQUIRE(root->children[0]->type == ELEMENT_AST_NODE_STRUCT);

        // Declaration
        REQUIRE(root->children[0]->children[0]->type == ELEMENT_AST_NODE_DECLARATION);
        REQUIRE(root->children[0]->children[0]->identifier == "MyStruct");

        // Portlist
        REQUIRE(root->children[0]->children[0]->children[0]->type == ELEMENT_AST_NODE_PORTLIST);
        REQUIRE(root->children[0]->children[0]->children[0]->children[0]->type == ELEMENT_AST_NODE_PORT);
        REQUIRE(root->children[0]->children[0]->children[0]->children[0]->identifier == "a");
        REQUIRE(root->children[0]->children[0]->children[0]->children[1]->type == ELEMENT_AST_NODE_PORT);
        REQUIRE(root->children[0]->children[0]->children[0]->children[1]->identifier == "b");

        // Implicit Return
        REQUIRE(root->children[0]->children[0]->children[1]->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
        REQUIRE(root->children[0]->children[0]->children[1]->parent->type == ELEMENT_AST_NODE_DECLARATION);

        // No body
        REQUIRE(root->children[0]->children[1]->type == ELEMENT_AST_NODE_NO_BODY);
    }

    SECTION("Example Code - Lambda") {
        element_tokeniser_run(tokeniser, "a = _(_a) = 5;" ,"<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        // Root
        REQUIRE(parser.root->type == ELEMENT_AST_NODE_ROOT);

        auto root = parser.root;
        print_ast(root, 0, nullptr);

        // Function
        REQUIRE(root->children[0]->type == ELEMENT_AST_NODE_FUNCTION);

        // Declaration
        REQUIRE(root->children[0]->children[0]->type == ELEMENT_AST_NODE_DECLARATION);
        REQUIRE(root->children[0]->children[0]->identifier == "a");

        // Empty Inputs
        REQUIRE(root->children[0]->children[0]->children[0]->type == ELEMENT_AST_NODE_NONE);
        REQUIRE(root->children[0]->children[0]->children[0]->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT));

        // Implicit Return
        REQUIRE(root->children[0]->children[0]->children[1]->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
        REQUIRE(root->children[0]->children[0]->children[1]->parent->type == ELEMENT_AST_NODE_DECLARATION);

        // Lambda
        REQUIRE(root->children[0]->children[1]->type == ELEMENT_AST_NODE_LAMBDA);

        // Portlist
        REQUIRE(root->children[0]->children[1]->children[0]->type == ELEMENT_AST_NODE_PORTLIST);
        REQUIRE(root->children[0]->children[1]->children[0]->children[0]->type == ELEMENT_AST_NODE_PORT);
        REQUIRE(root->children[0]->children[1]->children[0]->children[0]->identifier == "_a");

        // Literal
        REQUIRE(root->children[0]->children[1]->children[1]->type == ELEMENT_AST_NODE_LITERAL);
        REQUIRE(root->children[0]->children[1]->children[1]->literal == 5);
    }

    SECTION("Example Code - Namespace") {
        element_tokeniser_run(tokeniser, "namespace Empty {}" ,"<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        // Root
        REQUIRE(parser.root->type == ELEMENT_AST_NODE_ROOT);

        auto root = parser.root;
        print_ast(root, 0, nullptr);

        // Namespace
        REQUIRE(root->children[0]->type == ELEMENT_AST_NODE_NAMESPACE);
        REQUIRE(root->children[0]->identifier == "Empty");

        // Scope
        REQUIRE(root->children[0]->children[0]->type == ELEMENT_AST_NODE_SCOPE);

    }

    SECTION("Example Code - Intrinsic Constraint") {
        element_tokeniser_run(tokeniser, "intrinsic constraint Any;" ,"<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        // Root
        REQUIRE(parser.root->type == ELEMENT_AST_NODE_ROOT);

        auto root = parser.root;
        print_ast(root, 0, nullptr);

        // Constraint
        REQUIRE(root->children[0]->type == ELEMENT_AST_NODE_CONSTRAINT);

        // Intrinsic Declaration
        REQUIRE(root->children[0]->children[0]->type == ELEMENT_AST_NODE_DECLARATION);
        REQUIRE(root->children[0]->children[0]->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC));
        REQUIRE(root->children[0]->children[0]->identifier == "Any");

        // Empty Inputs
        REQUIRE(root->children[0]->children[0]->children[0]->type == ELEMENT_AST_NODE_NONE);
        REQUIRE(root->children[0]->children[0]->children[0]->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT));

        // Implicit Return
        REQUIRE(root->children[0]->children[0]->children[1]->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
        REQUIRE(root->children[0]->children[0]->children[1]->parent->type == ELEMENT_AST_NODE_DECLARATION);

        // No body
        REQUIRE(root->children[0]->children[1]->type == ELEMENT_AST_NODE_NO_BODY);

    }

    SECTION("Example Code - Function Call With Inputs") {
        element_tokeniser_run(tokeniser, "evaluate = Num.add(1, 2);" ,"<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        // Root
        REQUIRE(parser.root->type == ELEMENT_AST_NODE_ROOT);

        auto root = parser.root;
        print_ast(root, 0, nullptr);

        // Function
        REQUIRE(root->children[0]->type == ELEMENT_AST_NODE_FUNCTION);

        // Declaration
        REQUIRE(root->children[0]->children[0]->type == ELEMENT_AST_NODE_DECLARATION);
        REQUIRE(root->children[0]->children[0]->identifier == "evaluate");

        // Empty Inputs
        REQUIRE(root->children[0]->children[0]->children[0]->type == ELEMENT_AST_NODE_NONE);
        REQUIRE(root->children[0]->children[0]->children[0]->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT));

        // Implicit Return
        REQUIRE(root->children[0]->children[0]->children[1]->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
        REQUIRE(root->children[0]->children[0]->children[1]->parent->type == ELEMENT_AST_NODE_DECLARATION);

        // Call Num
        REQUIRE(root->children[0]->children[1]->type == ELEMENT_AST_NODE_CALL);
        REQUIRE(root->children[0]->children[1]->identifier == "Num");

        // Call add
        REQUIRE(root->children[0]->children[1]->children[0]->type == ELEMENT_AST_NODE_CALL);
        REQUIRE(root->children[0]->children[1]->children[0]->identifier == "add");

        // Expression List
        REQUIRE(root->children[0]->children[1]->children[1]->type == ELEMENT_AST_NODE_EXPRLIST);

        // Literals
        REQUIRE(root->children[0]->children[1]->children[1]->children[0]->type == ELEMENT_AST_NODE_LITERAL);
        REQUIRE(root->children[0]->children[1]->children[1]->children[0]->literal == 1);
        REQUIRE(root->children[0]->children[1]->children[1]->children[1]->type == ELEMENT_AST_NODE_LITERAL);
        REQUIRE(root->children[0]->children[1]->children[1]->children[1]->literal == 2);
    }

}