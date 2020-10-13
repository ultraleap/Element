#include <catch2/catch.hpp>

//STD
#include <array>

//LIBS
#include <fmt/format.h>

//SELF
#include "element/object.h"
#include "interpreter_internal.hpp"

#include "util.test.hpp"
#include "object_model/intermediaries/function_instance.hpp"
#include "object_model/intermediaries/struct_instance.hpp"

// Include specific tests for object model generation here
TEST_CASE("ObjectModel", "[API]")
{
    SECTION("Call")
    {
        element_declaration* const_int_declaration = nullptr;
        element_declaration* my_struct_declaration = nullptr;
        element_object* const_int_obj = nullptr;
        element_object* my_struct_obj = nullptr;
        element_interpreter_ctx* context = nullptr;

        const std::string package = "";

        element_interpreter_create(&context);
        element_interpreter_set_log_callback(context, log_callback, nullptr);
        element_interpreter_load_prelude(context);

        if (!package.empty())
        {
            const auto result = element_interpreter_load_package(context, package.c_str());
            REQUIRE(result == ELEMENT_OK);
        }

        std::array<char, 2048> output_buffer_array{};
        auto* output_buffer = output_buffer_array.data();

        const auto* input_element = "const_int = 5\n"
                            "struct my_struct(a:Num)\n"
                            "{\n"
                            "func(thing:my_struct, val:Num) = thing.a.mul(val)\n"
                            "}\n";

        auto result = element_interpreter_load_string(context, input_element, "<input>");

        constexpr auto args_count = 1;
        element_object* args[args_count];
        element_object* const_int = nullptr;
        element_object* my_struct_instance = nullptr;
        element_object* my_struct_instance_a = nullptr;
        std::shared_ptr<const element::instruction_constant> constant;

        if (result != ELEMENT_OK)
            goto cleanup;

        result = element_interpreter_find(context, "const_int", &const_int_declaration);
        if (result != ELEMENT_OK)
            goto cleanup;

        element_declaration_to_object(const_int_declaration, &const_int_obj);

        result = element_object_compile(context, const_int_obj, &const_int);
        CHECK(result == ELEMENT_OK);

        result = element_interpreter_find(context, "my_struct", &my_struct_declaration);
        if (result != ELEMENT_OK)
            goto cleanup;

        element_declaration_to_object(my_struct_declaration, &my_struct_obj);

        args[0] = const_int;

        result = element_object_call(context, my_struct_obj, *args, args_count, &my_struct_instance);
        CHECK(result == ELEMENT_OK);

        result = element_object_index(context, my_struct_instance, "a", &my_struct_instance_a);
        CHECK(result == ELEMENT_OK);

        element_inputs input;
        input.values = nullptr;
        input.count = 0;
        element_outputs output;
        float outputs[] = { 0 };
        output.values = outputs;
        output.count = 1;
        element_interpreter_evaluate(context, nullptr, my_struct_instance_a, &input, &output);
        REQUIRE(outputs[0] == 5);
    cleanup:
        element_delete_object(&my_struct_instance_a);
        element_delete_object(&my_struct_instance);
        element_delete_object(&my_struct_obj);
        element_delete_object(&const_int_obj);
        element_delete_object(&const_int);
        element_delete_declaration(&my_struct_declaration);
        element_delete_declaration(&const_int_declaration);
        element_interpreter_delete(&context);
    }
}