//STD
#include <iostream>
#include <array>

//LIBS
#include <catch2/catch.hpp>

//SELF
#include "element/ast.h"
#include "element/token.h"
#include "ast/ast_internal.hpp"
#include "ast/parser_internal.hpp"

void check_ast_to_code(std::string input)
{
    element_tokeniser_ctx* tokeniser;
    element_tokeniser_create(&tokeniser);

    element_parser_ctx parser;

    UNSCOPED_INFO(input.c_str());
    element_tokeniser_run(tokeniser, input.c_str(), "<input>");

    parser.tokeniser = tokeniser;
    parser.ast_build();
    auto* root = parser.root;

    std::array<char, 2048> output_buffer{};
    if (element_ast_to_string(root, nullptr, output_buffer.data(), output_buffer.size() == ELEMENT_ERROR_INVALID_SIZE))
    {
        printf("Output buffer too small");
        return;
    }

    printf("%s", output_buffer.data());
    UNSCOPED_INFO(output_buffer.data());

    std::string code = ast_to_code(root, nullptr);

    // TODO: Remove semi colon hack
    REQUIRE(code == input);
}

TEST_CASE("AST to Source Code"
          "[AST_CODE]")
{

    SECTION("Burger1 = 2.0")
    {
        check_ast_to_code("Burger1 = 2.0");
    }

    SECTION("Burger1(a) = 2.0")
    {
        check_ast_to_code("Burger1(a) = 2.0");
    }

    SECTION("Burger1(a:Num) = a")
    {
        check_ast_to_code("Burger1(a:Num) = a");
    }

    SECTION("struct MyStruct(a, b)")
    {
        check_ast_to_code("struct MyStruct(a, b)");
    }

    SECTION("a = _(_a) = 5.0")
    {
        check_ast_to_code("a = _(_a) = 5.0");
    }

    SECTION("namespace Empty { }")
    {
        check_ast_to_code("namespace Empty { }");
    }

    SECTION("intrinsic constraint Any")
    {
        check_ast_to_code("intrinsic constraint Any");
    }

    SECTION("evaluate = Num.add(1.0, 2.0)")
    {
        check_ast_to_code("evaluate = Num.add(1.0, 2.0)");
    }

    SECTION("discardedArg(_) = pi")
    {
        check_ast_to_code("discardedArg(_) = pi");
    }

    SECTION("e = bar(a)(b)(c)(foo)")
    {
        check_ast_to_code("e = bar(a)(b)(c)(foo)");
    }

    SECTION("constraint Predicate(a:Num):Bool")
    {
        check_ast_to_code("constraint Predicate(a:Num):Bool");
    }

    SECTION("namespace Blah { a = 5.0 b = 10.0 }")
    {
        check_ast_to_code("namespace Blah { a = 5.0 b = 10.0 }");
    }

    SECTION("function(a, b) { struct MyStruct(a, b) return = MyStruct(a, b) }")
    {
        check_ast_to_code("function(a, b) { struct MyStruct(a, b) return = MyStruct(a, b) }");
    }

    SECTION("c = -10.86")
    {
        check_ast_to_code("c = -10.86");
    }

    SECTION("invalidReturn(a:Num, b:Num):Num { c = Num.add(a.x, b.x) }")
    {
        check_ast_to_code("invalidReturn(a:Num, b:Num):Num { c = Num.add(a.x, b.x) }");
    }

    SECTION("scope_bodied { return = 5.0 }")
    {
        check_ast_to_code("scopeBodied { return = 5.0 }");
    }

    SECTION("Preserve Whitespace")
    {
        check_ast_to_code("struct myStruct(a:Num)\n"
                          "{\n"
                          "    a(b:Num) = b \n"
                          "}");
    }
}