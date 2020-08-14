#include <catch2/catch.hpp>
#include <iostream>

#include "element/ast.h"
#include "element/token.h"

#include "../src/ast/ast_internal.hpp"

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

    SECTION("Example Code - Function Call With Inputs") {
        element_tokeniser_run(tokeniser, "evaluate = Num.add(1, 2);" ,"<input>");

        parser.tokeniser = tokeniser;
        parser.ast_build();

        // Root
        REQUIRE(parser.root->type == ELEMENT_AST_NODE_ROOT);

        auto root = parser.root;

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
        REQUIRE(root->children[0]->children[1]->children[1]->children[0]->literal == 1);
        REQUIRE(root->children[0]->children[1]->children[1]->children[1]->literal == 2);
    }

}