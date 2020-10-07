#include <catch2/catch.hpp>

//STD
#include <array>

//LIBS
#include <fmt/format.h>

//SELF
#include "element/interpreter_object_model.h"

#include "util.test.hpp"

element_result compile_test(
    const char* input, 
    const char* to_compile, 
    const std::function<element_result(const element_object*)>& func,
    const std::string& package = "")
{
    element_interpreter_ctx* context = nullptr;
    element_declaration* declaration = nullptr;
    element_object* object = nullptr;

    element_interpreter_create(&context);
    element_interpreter_set_log_callback(context, log_callback, nullptr);
    element_interpreter_load_prelude(context);

    if (!package.empty())
    {
        const auto result = element_interpreter_load_package(context, package.c_str());
        if (result != ELEMENT_OK)
            return result;
    }

    std::array<char, 2048> output_buffer_array{};
    auto* output_buffer = output_buffer_array.data();

    auto result = element_interpreter_load_string(context, input, "<input>");
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_find(context, to_compile, &declaration);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_object_model_compile(context, nullptr, declaration, &object);
    if (result != ELEMENT_OK)
        goto cleanup;

    if (func)
        result = func(object);

cleanup:
    element_delete_declaration(&declaration);
    element_delete_object(&object);
    element_interpreter_delete(&context);
    return result;
}


// Include specific tests for object model generation here
TEST_CASE("ObjectModel", "[API]")
{
    SECTION("Compile")
    {
        auto check = [](const element_object* obj) -> element_result
        {

            return ELEMENT_OK;
        };

        element_result result = ELEMENT_OK;
        result = compile_test("struct my_struct(a:Num) { func(thing:my_struct) = thing.a.mul(2) }", "my_struct.func", check);
        REQUIRE(result == ELEMENT_OK);
    }

    SECTION("Call")
    {
    }

    SECTION("Index")
    {
    }
}