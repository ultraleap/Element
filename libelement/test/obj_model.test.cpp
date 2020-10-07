#include <catch2/catch.hpp>

//STD
#include <array>

//LIBS
#include <fmt/format.h>

//SELF
#include "element/interpreter_object_model.h"
#include "interpreter_internal.hpp"

#include "util.test.hpp"
#include "object_model/intermediaries/function_instance.hpp"

element_result find_declaration(
    const char* input,
    const char* identifier,
    const std::function<element_result(element_interpreter_ctx* , const element_declaration*, element_object**)>& func,
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

    result = element_interpreter_find(context, identifier, &declaration);
    if (result != ELEMENT_OK)
        goto cleanup;

    if (func)
        result = func(context, declaration, &object);

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
        auto compile = [](element_interpreter_ctx* context, const element_declaration* declaration, element_object** obj) -> element_result
        {
            const auto result = element_object_model_compile(context, nullptr, declaration, obj);
            if (result != ELEMENT_OK)
                return result;

            const auto function_instance = std::dynamic_pointer_cast<const element::function_instance>((*obj)->obj);
            if (function_instance)
                return ELEMENT_OK;

            return ELEMENT_ERROR_UNKNOWN;
        };

        auto result = ELEMENT_OK;
        result = find_declaration("struct my_struct(a:Num) { func(thing:my_struct) = thing.a.mul(2) }", "my_struct.func", compile);
        REQUIRE(result == ELEMENT_OK);
    }

    SECTION("Call")
    {


    }

    SECTION("Index")
    {
        auto index = [](element_interpreter_ctx* context, const element_declaration* declaration, element_object** obj) -> element_result {

            const auto result = element_object_model_index(context, nullptr, declaration, "func", obj);
            if (result != ELEMENT_OK)
                return result;

            const auto function_instance = std::dynamic_pointer_cast<const element::function_instance>((*obj)->obj);
            if (function_instance)
                return ELEMENT_OK;

            return ELEMENT_ERROR_UNKNOWN;
        };

        element_result result = ELEMENT_OK;
        result = find_declaration("struct my_struct(a:Num) { func(thing:my_struct) = thing.a.mul(2) }", "my_struct", index);
        REQUIRE(result == ELEMENT_OK);
    }
}