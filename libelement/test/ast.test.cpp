#include <catch2/catch.hpp>
#include <iostream>

#include "element/ast.h"
#include "element/token.h"
#include "../src/ast/ast_internal.hpp"

//STD
#include <array>
#include <cstring>

void print_ast(element_ast* ast, int depth, element_ast* ast_to_mark)
{
    std::array<char, 2048> output_buffer{};
    element_ast_to_string(ast, depth, ast_to_mark, output_buffer.data(), output_buffer.size());
    printf("%s", output_buffer.data());
    UNSCOPED_INFO(output_buffer.data());
}

TEST_CASE("Parser", "[AST]") {

    element_tokeniser_ctx* tokeniser;
    element_tokeniser_create(&tokeniser);

    element_parser_ctx parser;

    SECTION("Functions: Burger1 = 2") {
        const std::string input = "Burger1 = 2";
        UNSCOPED_INFO(input.c_str());
        element_tokeniser_run(tokeniser, input.c_str(), "<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        auto* root = parser.root;
        print_ast(root, 0, nullptr);

        // Root
        REQUIRE(root->type == ELEMENT_AST_NODE_ROOT);

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

        element_ast_delete(parser.root);
    }

    SECTION("Functions: Burger1(a) = 2") {
        const std::string input = "Burger1(a) = 2";
        UNSCOPED_INFO(input.c_str());
        element_tokeniser_run(tokeniser, input.c_str(), "<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        auto* root = parser.root;
        print_ast(root, 0, nullptr);

        // Root
        REQUIRE(root->type == ELEMENT_AST_NODE_ROOT);

        const auto& function = root->children[0];
        REQUIRE(function->type == ELEMENT_AST_NODE_FUNCTION);

        const auto& declaration = function->children[0];
        REQUIRE(declaration->type == ELEMENT_AST_NODE_DECLARATION);
        REQUIRE(declaration->identifier == "Burger1");

        const auto& portlist = declaration->children[0];
        REQUIRE(portlist->type == ELEMENT_AST_NODE_PORTLIST);
        REQUIRE(!portlist->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT));

        const auto& port = portlist->children[0];
        REQUIRE(port->type == ELEMENT_AST_NODE_PORT);
        REQUIRE(port->children.size() == 0);

        const auto& return_annotation = declaration->children[1];
        REQUIRE(return_annotation->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE);

        // Literal
        const auto& body = function->children[1];
        REQUIRE(body->type == ELEMENT_AST_NODE_LITERAL);
        REQUIRE(body->literal == 2);

        element_ast_delete(parser.root);
    }

    SECTION("Functions: Burger1(a:Num) = a") {
        const std::string input = "Burger1(a:Num) = a";
        UNSCOPED_INFO(input.c_str());
        element_tokeniser_run(tokeniser, input.c_str(), "<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        auto* root = parser.root;
        print_ast(root, 0, nullptr);

        REQUIRE(root->type == ELEMENT_AST_NODE_ROOT);

        const auto& function = root->children[0];
        REQUIRE(function->type == ELEMENT_AST_NODE_FUNCTION);

        const auto& declaration = function->children[0];
        REQUIRE(declaration->type == ELEMENT_AST_NODE_DECLARATION);
        REQUIRE(declaration->identifier == "Burger1");

        const auto& portlist = declaration->children[0];
        REQUIRE(portlist->type == ELEMENT_AST_NODE_PORTLIST);
        REQUIRE(!portlist->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT));

        const auto& port = portlist->children[0];
        REQUIRE(port->type == ELEMENT_AST_NODE_PORT);

        const auto& type_name = port->children[0];
        REQUIRE(type_name->type == ELEMENT_AST_NODE_TYPENAME);
        REQUIRE(type_name->children[0]->type == ELEMENT_AST_NODE_IDENTIFIER);
        REQUIRE(type_name->children[0]->identifier == "Num");

        const auto& return_annotation = declaration->children[1];
        REQUIRE(return_annotation->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE);

        const auto& body = function->children[1];
        REQUIRE(body->type == ELEMENT_AST_NODE_CALL);
        REQUIRE(body->identifier == "a");

        element_ast_delete(parser.root);
    }

    SECTION("Structs: struct MyStruct(a, b)") {
        const std::string input = "struct MyStruct(a, b)";
        UNSCOPED_INFO(input.c_str());
        element_tokeniser_run(tokeniser, input.c_str(), "<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        auto* root = parser.root;
        print_ast(root, 0, nullptr);

        // Root
        REQUIRE(root->type == ELEMENT_AST_NODE_ROOT);

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

        element_ast_delete(parser.root);
    }

    SECTION("Functions: a = _(_a) = 5") {
        const std::string input = "a = _(_a) = 5";
        UNSCOPED_INFO(input.c_str());
        element_tokeniser_run(tokeniser, input.c_str(), "<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        auto* root = parser.root;
        print_ast(root, 0, nullptr);

        // Root
        REQUIRE(root->type == ELEMENT_AST_NODE_ROOT);

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

        // Type
        REQUIRE(root->children[0]->children[1]->children[1]->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE);

        // Literal
        REQUIRE(root->children[0]->children[1]->children[2]->type == ELEMENT_AST_NODE_LITERAL);
        REQUIRE(root->children[0]->children[1]->children[2]->literal == 5);

        element_ast_delete(parser.root);
    }

    SECTION("Namespace: namespace Empty {}") {
        const std::string input = "namespace Empty {}";
        UNSCOPED_INFO(input.c_str());
        element_tokeniser_run(tokeniser, input.c_str(), "<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        auto* root = parser.root;
        print_ast(root, 0, nullptr);

        // Root
        REQUIRE(root->type == ELEMENT_AST_NODE_ROOT);

        // Namespace
        REQUIRE(root->children[0]->type == ELEMENT_AST_NODE_NAMESPACE);
        REQUIRE(root->children[0]->identifier == "Empty");

        // Scope
        REQUIRE(root->children[0]->children[0]->type == ELEMENT_AST_NODE_SCOPE);

        element_ast_delete(parser.root);
    }

    SECTION("Constraint: intrinsic constraint Any") {
        const std::string input = "intrinsic constraint Any";
        UNSCOPED_INFO(input.c_str());
        element_tokeniser_run(tokeniser, input.c_str(), "<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        auto* root = parser.root;
        print_ast(root, 0, nullptr);

        // Root
        REQUIRE(root->type == ELEMENT_AST_NODE_ROOT);

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

        element_ast_delete(parser.root);
    }

    SECTION("Functions: evaluate = Num.add(1, 2)") {
        const std::string input = "evaluate = Num.add(1, 2)";
        UNSCOPED_INFO(input.c_str());
        element_tokeniser_run(tokeniser, input.c_str(), "<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        auto* root = parser.root;
        print_ast(root, 0, nullptr);

        // Root
        REQUIRE(root->type == ELEMENT_AST_NODE_ROOT);

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

        element_ast_delete(parser.root);
    }

    element_tokeniser_delete(tokeniser);
}