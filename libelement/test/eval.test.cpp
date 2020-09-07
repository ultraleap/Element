#include <catch2/catch.hpp>

//STD
#include <array>
#include <cstring>

//LIBS
#include <fmt/format.h>

//SELF
#include "element/token.h"
#include "element/ast.h"
#include "element/interpreter.h"
#include "element/common.h"

#include "etree/fwd.hpp"
#include "etree/expressions.hpp"
#include "etree/evaluator.hpp"

void log_callback(const element_log_message* msg)
{
    char buffer[512];
    buffer[0] = '^';
    buffer[1] = '\0';
    const char* buffer_str = NULL;
    if (msg->character - 1 >= 0)
    {
        const int padding_count = msg->character - 1;
        for (int i = 0; i < padding_count; ++i)
        {
            buffer[i] = ' ';
        }

        int end = padding_count + msg->length;
        for (int i = padding_count; i < end; ++i)
        {
            buffer[i] = '^';
        }

        buffer[end] = '\0';

        buffer_str = &buffer[0];
    }

    std::array<char, 2048> output_buffer_array{};
    char* output_buffer = output_buffer_array.data();

    sprintf(output_buffer, "\n----------ELE%d %s\n%d| %s\n%d| %s\n\n%s\n----------\n\n",
        msg->message_code,
        msg->filename,
        msg->line,
        msg->line_in_source ? msg->line_in_source : "",
        msg->line,
        buffer_str,
        msg->message);

    printf("%s", output_buffer);
    UNSCOPED_INFO(output_buffer);
}

element_result eval(const char* evaluate)
{
    element_interpreter_ctx* context = NULL;
    element_declaration* declaration = NULL;
    element_object* object = NULL;

    element_interpreter_create(&context);
    element_interpreter_set_log_callback(context, log_callback);
    element_interpreter_load_prelude(context);

    float inputs[] = { 1, 2 };
    float outputs[1];

    element_inputs input;
    element_outputs output;

    std::array<char, 2048> output_buffer_array{};
    char* output_buffer = output_buffer_array.data();

    element_result result = element_interpreter_load_string(context, evaluate, "<input>");
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_find(context, "evaluate", &declaration);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_compile(context, NULL, declaration, &object);
    if (result != ELEMENT_OK)
        goto cleanup;

    input.values = inputs;
    input.count = 2;

    output.values = outputs;
    output.count = 1;

    result = element_interpreter_evaluate(context, NULL, object, &input, &output);
    if (result != ELEMENT_OK)
        goto cleanup;

    sprintf(output_buffer + strlen(output_buffer), "%s -> {", evaluate);
    for (int i = 0; i < output.count; ++i)
    {
        sprintf(output_buffer + strlen(output_buffer), "%f", output.values[i]);
        if (i != output.count - 1)
        {
            sprintf(output_buffer + strlen(output_buffer), ", ");
        }
    }
    sprintf(output_buffer + strlen(output_buffer), "}\n");

    printf("%s", output_buffer);
    UNSCOPED_INFO(output_buffer);

cleanup:
    element_delete_declaration(context, &declaration);
    element_delete_object(context, &object);
    element_interpreter_delete(context);
    return result;
}

element_result eval_with_source(const char* source, const char* evaluate)
{
    element_interpreter_ctx* context = NULL;
    element_declaration* declaration = NULL;
    element_object* object = NULL;

    element_interpreter_create(&context);
    element_interpreter_set_log_callback(context, log_callback);
    element_interpreter_load_prelude(context);

    float inputs[] = { 1, 2 };
    float outputs[1];

    element_inputs input;
    element_outputs output;

    std::array<char, 2048> output_buffer_array{};
    char* output_buffer = output_buffer_array.data();

    element_result result = element_interpreter_load_string(context, source, "<source>");
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_load_string(context, evaluate, "<input>");
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_find(context, "evaluate", &declaration);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_compile(context, NULL, declaration, &object);
    if (result != ELEMENT_OK)
        goto cleanup;

    input.values = inputs;
    input.count = 2;

    output.values = outputs;
    output.count = 1;

    result = element_interpreter_evaluate(context, NULL, object, &input, &output);
    if (result != ELEMENT_OK)
        goto cleanup;

    sprintf(output_buffer + strlen(output_buffer), "%s -> {", evaluate);
    for (int i = 0; i < output.count; ++i)
    {
        sprintf(output_buffer + strlen(output_buffer), "%f", output.values[i]);
        if (i != output.count - 1)
        {
            sprintf(output_buffer + strlen(output_buffer), ", ");
        }
    }
    sprintf(output_buffer + strlen(output_buffer), "}\n");

    printf("%s", output_buffer);
    UNSCOPED_INFO(output_buffer);

cleanup:
    element_delete_declaration(context, &declaration);
    element_delete_object(context, &object);
    element_interpreter_delete(context);
    return result;
}

element_result eval_with_inputs(const char* evaluate, element_inputs* inputs, element_outputs* outputs, std::string package = "")
{
    element_interpreter_ctx* context = NULL;
    element_declaration* declaration = NULL;
    element_object* object = NULL;

    element_interpreter_create(&context);
    element_interpreter_set_log_callback(context, log_callback);
    element_interpreter_load_prelude(context);

    if (!package.empty())
    {
        auto result = element_interpreter_load_package(context, package.c_str());
        if (result != ELEMENT_OK)
            return result;
    }

    std::array<char, 2048> output_buffer_array{};
    char* output_buffer = output_buffer_array.data();

    element_result result = element_interpreter_load_string(context, evaluate, "<input>");
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_find(context, "evaluate", &declaration);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_compile(context, NULL, declaration, &object);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_evaluate(context, NULL, object, inputs, outputs);
    if (result != ELEMENT_OK)
        goto cleanup;

    sprintf(output_buffer + strlen(output_buffer), "%s -> {", evaluate);
    for (int i = 0; i < outputs->count; ++i)
    {
        sprintf(output_buffer + strlen(output_buffer), "%f", outputs->values[i]);
        if (i != outputs->count - 1)
        {
            sprintf(output_buffer + strlen(output_buffer), ", ");
        }
    }
    sprintf(output_buffer + strlen(output_buffer), "}\n");

    printf("%s", output_buffer);
    UNSCOPED_INFO(output_buffer);

cleanup:
    element_delete_declaration(context, &declaration);
    element_delete_object(context, &object);
    element_interpreter_delete(context);
    return result;
}

TEST_CASE("Interpreter", "[Evaluate]")
{
    SECTION("Runtime evaluation")
    {
        element_result result = ELEMENT_OK;

        SECTION("Invalid Boundary Function")
        {
            SECTION("Error - Missing Return Annotation")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("evaluate(a:Num) = a.mul(2);", &input, &output);
                REQUIRE(result == ELEMENT_ERROR_INVALID_BOUNDARY_FUNCTION_INTERFACE);
            }

            SECTION("Error - Missing Port Annotation")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("evaluate(a):Num = a.mul(2);", &input, &output);
                REQUIRE(result == ELEMENT_ERROR_INVALID_BOUNDARY_FUNCTION_INTERFACE);
            }

            SECTION("Error - Port Annotation is Any")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("evaluate(a:Any):Num = a.mul(2);", &input, &output);
                REQUIRE(result == ELEMENT_ERROR_INVALID_BOUNDARY_FUNCTION_INTERFACE);
            }

            SECTION("Error - Return Annotation is Any")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("evaluate(a:Num):Any = a.mul(2);", &input, &output);
                REQUIRE(result == ELEMENT_ERROR_INVALID_BOUNDARY_FUNCTION_INTERFACE);
            }

            SECTION("Error - Return Annotation is Namespace")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("namespace Namespace{} evaluate(a:Num):Namespace = Namespace;", &input, &output);
                REQUIRE(result == ELEMENT_ERROR_INVALID_BOUNDARY_FUNCTION_INTERFACE);
            }

            SECTION("Error - Returns HigherOrder Function")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("myMul(a, b) = a.mul(b); evaluate(a:Num, b:Num):myMul = myMul;", &input, &output);
                REQUIRE(result == ELEMENT_ERROR_INVALID_BOUNDARY_FUNCTION_INTERFACE);
            }

            SECTION("Error - Returns Struct containing HigherOrder Function")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("myMul(a, b) = a.mul(b); struct myStruct(a:myMul) {} evaluate(a:Num, b:Num):myStruct = myStruct(myMul);", &input, &output);
                REQUIRE(result == ELEMENT_ERROR_INVALID_BOUNDARY_FUNCTION_INTERFACE);
            }

            SECTION("Error - Port is HigherOrder Function")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("myMul(a, b) = a.mul(b); evaluate(a:myMul):Num = a(1, 2);", &input, &output);
                REQUIRE(result == ELEMENT_ERROR_INVALID_BOUNDARY_FUNCTION_INTERFACE);
            }

            SECTION("Error - Port is Struct containing HigherOrder Function")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("myMul(a, b) = a.mul(b); struct myStruct(a:myMul) {}  evaluate(a:myStruct):Num = a.a(1, 2);", &input, &output);
                REQUIRE(result == ELEMENT_ERROR_INVALID_BOUNDARY_FUNCTION_INTERFACE);
            }
        }

        SECTION("Constraints")
        {
            SECTION("Unspecified annotations")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("returnIt(a) = a; evaluate(a:Num):Num = returnIt(a);", &input, &output);
                REQUIRE(result == ELEMENT_OK);
                REQUIRE(outputs[0] == inputs[0]);
            }
        }

        SECTION("HigherOrderFunctions")
        {
            SECTION("Apply Num.add to number")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("doIt(a:Num, b:Binary):Num = b(a, 1); evaluate(a:Num):Num = doIt(a, Num.add);", &input, &output);
                REQUIRE(result == ELEMENT_OK);
                REQUIRE(outputs[0] == inputs[0] + 1.0f);
            }

            SECTION("Apply lambda to number")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("do(func, param) = func(param); evaluate(a:Num):Num = do(_(b) = b, a);", &input, &output);
                REQUIRE(result == ELEMENT_OK);
                REQUIRE(outputs[0] == inputs[0]);
            }

            SECTION("Apply nested scoped lambdas to number")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs(
                    "do(func, param) = func(param)(param);"
                    "evaluate(a:Num):Num = do(_(b) {"
                    "    return = _(c) {"
                    "        mul(a, b) = a.mul(b);"
                    "        return = mul(c.mul(b), 2);"
                    "    };"
                    "}, a);",
                    &input, &output);
                REQUIRE(result == ELEMENT_OK);
                REQUIRE(outputs[0] == inputs[0] * inputs[0] * 2.0f);
            }

            SECTION("Apply nested scoped lambdas and expression lambda to number")
            {
                float inputs[] = { 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs(
                    "do(func, func2, param) = func2(func(param)(param));"
                    "evaluate(a:Num):Num = do(_(b) {"
                    "    return = _(c) {"
                    "        mul(a, b) = a.mul(b);"
                    "        return = mul(c.mul(b), 2);"
                    "    };"
                    "}, _(d) = d.mul(2), a);",
                    &input, &output);
                REQUIRE(result == ELEMENT_OK);
                REQUIRE(outputs[0] == inputs[0] * inputs[0] * 2.0f * 2.0f);
            }


            SECTION("Nullary function returning lambda")
            {
                float inputs[] = { 1 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("nullary = _(a:Num):Num = a.mul(10); evaluate(a:Num):Num = nullary(a);", &input, &output);
                REQUIRE(result == ELEMENT_OK);
                REQUIRE(outputs[0] == inputs[0] * 10.0f);
            }


            SECTION("Nullary function returning struct containing lambda")
            {
                float inputs[] = { 1 };
                element_inputs input;
                input.values = inputs;
                input.count = 1;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("constraint func_constraint(a); struct container(contained:func_constraint); nullary = container(_(a) = a.mul(10)); evaluate(a:Num):Num = nullary.contained(a);", &input, &output);
                REQUIRE(result == ELEMENT_OK);
                REQUIRE(outputs[0] == inputs[0] * 10.0f);
            }


            SECTION("Nullary function returning struct containing nested lambdas")
            {
                float inputs[] = { 1, 10 };
                element_inputs input;
                input.values = inputs;
                input.count = 2;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("constraint func_constraint(a); struct container(contained:func_constraint); nullary = container(_(a) = _(b) = a.mul(b)); evaluate(a:Num, b:Num):Num = nullary.contained(a)(b);", &input, &output);
                REQUIRE(result == ELEMENT_OK);
                REQUIRE(outputs[0] == inputs[0] * inputs[1]);
            }


            SECTION("Nullary function returning struct containing set of lambdas")
            {
                float inputs[] = { 1, 2 };
                element_inputs input;
                input.values = inputs;
                input.count = 2;
                element_outputs output;
                float outputs[] = { 0, 0 };
                output.values = outputs;
                output.count = 2;
                result = eval_with_inputs("constraint func_constraint(a); struct result(a:Num, b:Num); struct container(contained0:func_constraint, contained1:func_constraint); nullary = container(_(a) = a.mul(10), _(b) = b.mul(5)); evaluate(a:Num, b:Num):result = result(nullary.contained0(a), nullary.contained1(b));", &input, &output);
                REQUIRE(result == ELEMENT_OK);
                REQUIRE(outputs[0] == inputs[0] * 10.0f);
                REQUIRE(outputs[1] == inputs[1] * 5.0f);
            }


            SECTION("Nullary function returning struct containing set of nested lambdas")
            {
                float inputs[] = { 1, 10 };
                element_inputs input;
                input.values = inputs;
                input.count = 2;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("constraint func_constraint(a); struct container(contained:func_constraint); nullary = container(_(a) = _(b) = a.mul(b)); evaluate(a:Num, b:Num):Num = nullary.contained(a)(b);", &input, &output);
                REQUIRE(result == ELEMENT_OK);
                REQUIRE(outputs[0] == inputs[0] * inputs[1]);
            }

            SECTION("Nullary function returning struct containing multiple nested lambdas")
            {
                float inputs[] = { 1, 10 };
                element_inputs input;
                input.values = inputs;
                input.count = 2;
                element_outputs output;
                float outputs[] = { 0 };
                output.values = outputs;
                output.count = 1;
                result = eval_with_inputs("constraint func_constraint(a); struct container(one:func_constraint, two:func_constraint); nullary = container(_(a) = _(b) = a.mul(b), _(a) = _(b) = a.mul(b)); evaluate(a:Num, b:Num):Bool = Num.eq(nullary.one(a)(b), nullary.two(a)(b));", &input, &output);
                REQUIRE(result == ELEMENT_OK);
                REQUIRE(outputs[0] >= 0.0f);
            }
        }

        SECTION("Structs")
        {
            SECTION("Multiple inputs and outputs")
            {
                float inputs[] = { 2, 2, 3, 4 };
                element_inputs input;
                input.values = inputs;
                input.count = 4;
                element_outputs output;
                float outputs[] = { 0, 0, 0 };
                output.values = outputs;
                output.count = 3;

                char source[] =
                    "struct Vector3(x:Num, y:Num, z:Num) {}\n"
                    "struct Quaternion(scalar:Num, vector:Vector3) {}\n"
                    "evaluate(q:Quaternion):Vector3\n"
                    "{\n"
                    "    scale(vec:Vector3, s:Num) = Vector3(vec.x.mul(s), vec.y.mul(s), vec.z.mul(s));\n"
                    "    return = scale(q.vector, q.scalar);\n"
                    "}\n";

                result = eval_with_inputs(source, &input, &output);

                REQUIRE(result == ELEMENT_OK);

                REQUIRE(output.values[0] == input.values[0] * input.values[1]);
                REQUIRE(output.values[1] == input.values[0] * input.values[2]);
                REQUIRE(output.values[2] == input.values[0] * input.values[3]);
            }
        }

        SECTION("Prelude")
        {
            SECTION("Number")
            {

                SECTION("Double using input")
                {
                    float inputs[] = { 2 };
                    element_inputs input;
                    input.values = inputs;
                    input.count = 1;
                    element_outputs output;
                    float outputs[] = { 0 };
                    output.values = outputs;
                    output.count = 1;
                    result = eval_with_inputs("evaluate(a:Num):Num = a.mul(2);", &input, &output);
                    REQUIRE(result == ELEMENT_OK);
                    REQUIRE(output.values[0] == input.values[0] * 2);
                }

                SECTION("Multiply using inputs")
                {
                    float inputs[] = { 20, 10 };
                    element_inputs input;
                    input.values = inputs;
                    input.count = 2;
                    element_outputs output;
                    float outputs[] = { 0 };
                    output.values = outputs;
                    output.count = 1;
                    result = eval_with_inputs("evaluate(a:Num, b:Num):Num = a.mul(b);", &input, &output);
                    REQUIRE(result == ELEMENT_OK);
                    REQUIRE(output.values[0] == input.values[0] * input.values[1]);
                }

            }

            SECTION("Bool")
            {
                SECTION("if true, return expression")
                {
                    float inputs[] = { 1 };
                    element_inputs input;
                    input.values = inputs;
                    input.count = 1;
                    element_outputs output;
                    float outputs[] = { 0 };
                    output.values = outputs;
                    output.count = 1;

                    char source[] = "evaluate(a:Num):Num = Bool.if(Bool(a), a.mul(2), a);";

                    result = eval_with_inputs(source, &input, &output);

                    REQUIRE(result == ELEMENT_OK);
                    REQUIRE(output.values[0] == input.values[0] * 2);
                }

                SECTION("if true, return intermediary")
                {
                    float inputs[] = { 1 };
                    element_inputs input;
                    input.values = inputs;
                    input.count = 1;
                    element_outputs output;
                    float outputs[] = { 0, 0 };
                    output.values = outputs;
                    output.count = 2;

                    char source[] = "evaluate(a:Num):Vector2 = Bool.if(Bool(a), Vector2(a.mul(2), a.mul(2)), Vector2(a.mul(4), a.mul(4)));";

                    result = eval_with_inputs(source, &input, &output, "StandardLibrary");

                    REQUIRE(result == ELEMENT_OK);
                    REQUIRE(output.values[0] == 2);
                    REQUIRE(output.values[1] == 2);
                }
            }

            SECTION("List")
            {
                SECTION("at")
                {
                    SECTION("Error - Nullary list, Constant Index")
                    {
                        float inputs[] = { -1 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 1;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        char source[] = "evaluate(a:Num):Num = list.at(0);";
                        result = eval_with_inputs(source, &input, &output);

                        REQUIRE(result != ELEMENT_OK);
                    }

                    SECTION("Error - Nullary list, Runtime Index")
                    {
                        float inputs[] = { -1 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 1;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        char source[] = "evaluate(a:Num):Num = list.at(a);";
                        result = eval_with_inputs(source, &input, &output);

                        REQUIRE(result != ELEMENT_OK);
                    }

                    SECTION("list(1, 2, 3).at(<-1...3>)")
                    {
                        std::array<element_value, 10> inputs{ 1, 2, 3 };
                        element_inputs input;
                        input.values = inputs.data();
                        input.count = inputs.size();

                        element_outputs output;
                        std::array<element_value, 2> outputs{ 0, 0 };
                        output.values = outputs.data();
                        output.count = outputs.size();

                        std::string src = "evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).at({});";

                        SECTION("Negative Constant Index")
                        {
                            auto new_src = fmt::format(src, -1);
                            result = eval_with_inputs(new_src.c_str(), &input, &output);

                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == input.values[0]);
                        }

                        SECTION("Valid Constant Index")
                        {
                            for (int i = 0; i < 3; ++i)
                            {
                                auto new_src = fmt::format(src, i);
                                result = eval_with_inputs(new_src.c_str(), &input, &output);

                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(output.values[0] == input.values[i]);
                            }
                        }

                        SECTION("Beyond Length Constant Index")
                        {
                            auto new_src = fmt::format(src, 3);
                            result = eval_with_inputs(new_src.c_str(), &input, &output);

                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == input.values[2]);
                        }
                    }

                    SECTION("list(a, b, c).at(idx)")
                    {
                        std::array<element_value, 10> inputs{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
                        element_inputs input;
                        input.values = inputs.data();
                        input.count = inputs.size();

                        element_outputs output;
                        std::array<element_value, 2> outputs{ 0, 0 };
                        output.values = outputs.data();
                        output.count = outputs.size();

                        SECTION("Error - Mixed Element Types, Runtime Index")
                        {
                            inputs[6] = -1;
                            result = eval_with_inputs("evaluate(a:Num, b:Bool, c:Num, idx:Num):Num = list(a, b, c).at(idx);", &input, &output, "StandardLibrary");
                            REQUIRE(result != ELEMENT_OK);

                            inputs[6] = 0;
                            result = eval_with_inputs("evaluate(a:Bool, b:Num, c:Bool, idx:Num):Bool = list(a, b, c).at(idx);", &input, &output, "StandardLibrary");
                            REQUIRE(result != ELEMENT_OK);

                            inputs[6] = 3;
                            result = eval_with_inputs("evaluate(a:Num, b:Num, c:Bool, idx:Num):Num = Num(list(a, b, c).at(idx));", &input, &output, "StandardLibrary");
                            REQUIRE(result != ELEMENT_OK);
                        }

                        SECTION(" Mixed Element Types, Constant Index")
                        {
                            result = eval_with_inputs("evaluate(a:Num, b:Bool, c:Num, idx:Num):Num = list(a, b, c).at(-1);", &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[0]);

                            result = eval_with_inputs("evaluate(a:Bool, b:Num, c:Bool, idx:Num):Bool = list(a, b, c).at(0);", &input, &output, "StandardLibrary");
                            REQUIRE(outputs[0] == inputs[0]);

                            result = eval_with_inputs("evaluate(a:Num, b:Num, c:Bool, idx:Num):Num = Num(list(a, b, c).at(3));", &input, &output, "StandardLibrary");
                            REQUIRE(outputs[0] == inputs[2]);
                        }

                        SECTION("Runtime Intermediary Elements, Runtime Index")
                        {
                            SECTION("Homogenous Structs")
                            {
                                inputs[0] = 1;
                                inputs[1] = 1;
                                inputs[2] = 2;
                                inputs[3] = 2;
                                inputs[4] = 3;
                                inputs[5] = 3;

                                const char* src = "evaluate(a:Vector2, b:Vector2, c:Vector2, idx:Num):Vector2 = list(a, b, c).at(idx);";

                                SECTION("Negative Index")
                                {
                                    inputs[6] = -1;
                                    result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == inputs[0]);
                                    REQUIRE(outputs[1] == inputs[1]);
                                }

                                SECTION("Valid Index")
                                {
                                    for (int i = 0; i < 3; ++i)
                                    {
                                        inputs[6] = i;
                                        result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                                        REQUIRE(result == ELEMENT_OK);
                                        REQUIRE(outputs[0] == inputs[i * 2]);
                                        REQUIRE(outputs[1] == inputs[i * 2 + 1]);
                                    }
                                }

                                SECTION("Beyond Length Index")
                                {
                                    inputs[6] = 3;
                                    result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == inputs[4]);
                                    REQUIRE(outputs[1] == inputs[5]);
                                }
                            }

                            SECTION("Non-Homogenous Structs")
                            {
                                inputs[0] = 1;
                                inputs[1] = 1;
                                inputs[2] = 2;
                                inputs[3] = 2;
                                inputs[4] = 3;
                                inputs[5] = 3;
                                inputs[6] = 3;

                                const char* src = "evaluate(a:Vector2, b:Vector2, c:Vector3, idx:Num):Num = list(a, b, c).at(idx).x;";

                                SECTION("Negative Index")
                                {
                                    inputs[7] = -1;
                                    result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                                    REQUIRE(result != ELEMENT_OK);
                                }

                                SECTION("Valid Index")
                                {
                                    for (int i = 0; i < 3; ++i)
                                    {
                                        inputs[7] = i;
                                        result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                                        REQUIRE(result != ELEMENT_OK);
                                    }
                                }

                                SECTION("Beyond Length Index")
                                {
                                    inputs[7] = 3;
                                    result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                                    REQUIRE(result != ELEMENT_OK);
                                }
                            }
                        }

                        SECTION("Runtime Intermediary Elements, Constant Index")
                        {
                            SECTION("Homogenous Structs")
                            {
                                inputs[0] = 1;
                                inputs[1] = 1;
                                inputs[2] = 2;
                                inputs[3] = 2;
                                inputs[4] = 3;
                                inputs[5] = 3;

                                inputs[6] = -1;

                                const char* src = "evaluate(a:Vector2, b:Vector2, c:Vector2, idx:Num):Vector2 = list(a, b, c).at({});";

                                SECTION("Negative Index")
                                {
                                    std::string new_src = fmt::format(src, -1);
                                    result = eval_with_inputs(new_src.c_str(), &input, &output, "StandardLibrary");
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == inputs[0]);
                                    REQUIRE(outputs[1] == inputs[1]);
                                }

                                SECTION("Valid Index")
                                {
                                    for (int i = 0; i < 3; ++i)
                                    {
                                        std::string new_src = fmt::format(src, i);
                                        result = eval_with_inputs(new_src.c_str(), &input, &output, "StandardLibrary");
                                        REQUIRE(result == ELEMENT_OK);
                                        REQUIRE(outputs[0] == inputs[i * 2]);
                                        REQUIRE(outputs[1] == inputs[i * 2 + 1]);
                                    }
                                }

                                SECTION("Beyond Length Index")
                                {
                                    std::string new_src = fmt::format(src, 3);
                                    result = eval_with_inputs(new_src.c_str(), &input, &output, "StandardLibrary");
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == inputs[4]);
                                    REQUIRE(outputs[1] == inputs[5]);
                                }
                            }

                            SECTION("Non-Homogenous Structs")
                            {
                                inputs[0] = 1;
                                inputs[1] = 1;
                                inputs[2] = 2;
                                inputs[3] = 2;
                                inputs[4] = 3;
                                inputs[5] = 3;
                                inputs[6] = 3;

                                const char* src = "evaluate(a:Vector2, b:Vector2, c:Vector3, idx:Num):Num = list(a, b, c).at({}).x;";

                                SECTION("Negative Index")
                                {
                                    std::string new_src = fmt::format(src, -1);
                                    result = eval_with_inputs(new_src.c_str(), &input, &output, "StandardLibrary");
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == inputs[0]);
                                }

                                SECTION("Valid Index")
                                {
                                    for (int i = 0; i < 3; ++i)
                                    {
                                        std::string new_src = fmt::format(src, i);
                                        result = eval_with_inputs(new_src.c_str(), &input, &output, "StandardLibrary");
                                        REQUIRE(result == ELEMENT_OK);
                                        REQUIRE(outputs[0] == inputs[i * 2]);
                                    }
                                }

                                SECTION("Beyond Length Index")
                                {
                                    std::string new_src = fmt::format(src, 3);
                                    result = eval_with_inputs(new_src.c_str(), &input, &output, "StandardLibrary");
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == inputs[4]);
                                }
                            }
                        }

                        SECTION("Runtime Expression Elements, Runtime Index")
                        {
                            inputs[0] = 1;
                            inputs[1] = 2;
                            inputs[2] = 3;

                            const char* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = list(a, Num.add(0, b), a.mul(c)).at(idx);";

                            SECTION("Negative Index")
                            {
                                inputs[3] = -1;

                                result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[0]);
                            }

                            SECTION("Valid Index")
                            {
                                for (int i = 0; i < 3; ++i)
                                {
                                    inputs[3] = i;

                                    result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == inputs[i]);
                                }
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[3] = 3;

                                result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }
                        }

                        SECTION("Runtime Expression Elements, Constant Index")
                        {
                            inputs[0] = 1;
                            inputs[1] = 2;
                            inputs[2] = 3;

                            const char* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = list(a, Num.add(0, b), a.mul(c)).at({});";

                            SECTION("Negative Index")
                            {
                                std::string new_src = fmt::format(src, -1);
                                result = eval_with_inputs(new_src.c_str(), &input, &output, "StandardLibrary");
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[0]);
                            }

                            SECTION("Valid Index")
                            {
                                for (int i = 0; i < 3; ++i)
                                {
                                    std::string new_src = fmt::format(src, i);
                                    result = eval_with_inputs(new_src.c_str(), &input, &output, "StandardLibrary");
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == inputs[i]);
                                }
                            }

                            SECTION("Beyond Length Index")
                            {
                                std::string new_src = fmt::format(src, 3);
                                result = eval_with_inputs(new_src.c_str(), &input, &output, "StandardLibrary");
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }
                        }
                    }
                }

                SECTION("map")
                {
                    SECTION("list(1, 2, a).map(_(n:Num) = n.mul(b)).at(idx)")
                    {
                        std::array<float, 3> inputs = { 4, 2, 0 };
                        std::array<float, 1> outputs = { 0 };
                        element_inputs input{ inputs.data(), inputs.size() };
                        element_outputs output{ outputs.data(), outputs.size() };

                        const char* src = "evaluate(a:Num, b:Num, idx:Num):Num = list(1, 2, a).map(_(n:Num) = n.mul(b)).at(idx);";

                        SECTION("Negative Index")
                        {
                            inputs[2] = -1;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == 1 * inputs[1]);
                        }

                        SECTION("Valid Index")
                        {
                            inputs[2] = 0;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == 1 * inputs[1]);

                            inputs[2] = 1;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == 2 * inputs[1]);

                            inputs[2] = 2;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == inputs[0] * inputs[1]);
                        }

                        SECTION("Beyond Length Index")
                        {
                            inputs[2] = 3;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == inputs[0] * inputs[1]);
                        }

                        SECTION("NaN Index")
                        {
                            inputs[2] = std::nanf("");
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == 1 * inputs[1]);
                        }

                        SECTION("Max Value Index")
                        {
#undef max
                            inputs[2] = std::numeric_limits<float>::max();
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == inputs[0] * inputs[1]);
                        }

                        SECTION("PositiveInfinity Index")
                        {
                            inputs[2] = std::numeric_limits<float>::infinity();
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == inputs[0] * inputs[1]);
                        }

                        SECTION("NegativeInfinity Index")
                        {
                            inputs[2] = std::numeric_limits<float>::infinity() * -1.0f;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == 1 * inputs[1]);
                        }
                    }

                    SECTION("list(vec2, vec3).map(_(n) = n.magnitude).at(idx)")
                    {
                        std::array<float, 4> inputs = { 4, 2, 1, 0 };
                        std::array<float, 1> outputs = { 0 };
                        element_inputs input{ inputs.data(), inputs.size() };
                        element_outputs output{ outputs.data(), outputs.size() };

                        const char* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = list(Vector2(a, b), Vector3(c, b, a)).map(_(n) = n.magnitudeSquared).at(idx);";

                        SECTION("Negative Index")
                        {
                            inputs[3] = -1;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == inputs[0] * inputs[0] + inputs[1] + inputs[1]);
                        }

                        SECTION("Valid Index")
                        {
                            inputs[3] = 0;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == inputs[0] * inputs[0] + inputs[1] + inputs[1]);

                            inputs[3] = 1;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == inputs[0] * inputs[0] + inputs[1] + inputs[1] + inputs[2] * inputs[2]);
                        }

                        SECTION("Beyond Length Index")
                        {
                            inputs[3] = 2;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == inputs[0] * inputs[0] + inputs[1] + inputs[1] + inputs[2] * inputs[2]);
                        }

                        SECTION("NaN Index")
                        {
                            inputs[3] = std::nanf("");
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == inputs[0] * inputs[0] + inputs[1] + inputs[1]);
                        }

                        SECTION("Max Value Index")
                        {
#undef max
                            inputs[3] = std::numeric_limits<float>::max();
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == inputs[0] * inputs[0] + inputs[1] + inputs[1] + inputs[2] * inputs[2]);
                        }

                        SECTION("PositiveInfinity Index")
                        {
                            inputs[3] = std::numeric_limits<float>::infinity();
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == inputs[0] * inputs[0] + inputs[1] + inputs[1] + inputs[2] * inputs[2]);
                        }

                        SECTION("NegativeInfinity Index")
                        {
                            inputs[3] = std::numeric_limits<float>::infinity() * -1.0f;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(output.values[0] == inputs[0] * inputs[0] + inputs[1] + inputs[1]);
                        }
                    }

                    SECTION("list(vec2, vec3).map(_(n) = n.magnitude).at(0)")
                    {
                        float inputs[] = { 4, 2, 1 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(Vector2(a, b), Vector3(c, b, a)).map(_(n) = n.magnitudeSquared).at(0);", &input, &output, "StandardLibrary");
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(output.values[0] == inputs[0] * inputs[0] + inputs[1] + inputs[1]);
                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(Vector2(a, b), Vector3(c, b, a)).map(_(n) = n.magnitudeSquared).at(1);", &input, &output, "StandardLibrary");
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(output.values[0] == inputs[0] * inputs[0] + inputs[1] + inputs[1] + inputs[2] * inputs[2]);
                    }
                }

                SECTION("zip")
                {
                    SECTION("List.zip(list(a, b, c), list(3, 2, 1, 0), Num.add).at(idx)")
                    {
                        std::array<float, 4> inputs = { 1, 2, 3, 0 };
                        std::array<float, 1> outputs = { 0 };
                        element_inputs input{ inputs.data(), inputs.size() };
                        element_outputs output{ outputs.data(), outputs.size() };

                        const auto* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = List.zip(list(a, b, c), list(3, 2, 1, 0), Num.add)";

                        SECTION("Runtime Expression List, Runtime Index")
                        {
                            SECTION("Length Equals Smallest List")
                            {
                                auto new_src = fmt::format("{}.count;", src);
                                result = eval_with_inputs(new_src.c_str(), &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 3);
                            }
                        }
                    }

                    SECTION("List.zip(list(a, b, c, 0), list(3, 2, 1), Num.add).at(idx)")
                    {
                        std::array<float, 4> inputs = { 1, 2, 3, 0 };
                        std::array<float, 1> outputs = { 0 };
                        element_inputs input{ inputs.data(), inputs.size() };
                        element_outputs output{ outputs.data(), outputs.size() };

                        const auto* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = List.zip(list(a, b, c, 0), list(3, 2, 1), Num.add)";

                        SECTION("Runtime Expression List, Runtime Index")
                        {
                            SECTION("Length Equals Smallest List")
                            {
                                auto new_src = fmt::format("{}.count;", src);
                                result = eval_with_inputs(new_src.c_str(), &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 3);
                            }
                        }
                    }

                    SECTION("List.zip(list(a, b, c), list(3, 2, 1), Num.add).at(idx)")
                    {
                        std::array<float, 4> inputs = { 1, 2, 3, 0 };
                        std::array<float, 1> outputs = { 0 };
                        element_inputs input{ inputs.data(), inputs.size() };
                        element_outputs output{ outputs.data(), outputs.size() };

                        const auto* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = List.zip(list(a, b, c), list(3, 2, 1), Num.add).at(idx);";

                        SECTION("Runtime Expression List, Runtime Index")
                        {
                            SECTION("Negative Index")
                            {
                                inputs[3] = -1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 4);
                            }

                            SECTION("Valid Index")
                            {
                                inputs[3] = 0;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 4);
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[3] = 100;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 4);
                            }

                            SECTION("PositiveInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity();
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 4);
                            }

                            SECTION("NegativeInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity() * -1.0f;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 4);
                            }

                            SECTION("NaN Index")
                            {
                                inputs[3] = std::nanf("");
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == std::nanf(""));
                            }

                            SECTION("PositiveInfinity List Item")
                            {
                                inputs[0] = std::numeric_limits<float>::infinity();
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == std::numeric_limits<float>::infinity());
                            }

                            SECTION("NegativeInfinity List Item")
                            {
                                inputs[0] = std::numeric_limits<float>::infinity() * -1.0f;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == std::numeric_limits<float>::infinity() * -1.0f);
                            }

                            SECTION("NaN List Item")
                            {
                                inputs[0] = std::nanf("");
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == std::nanf(""));
                            }
                        }
                    }
                }

                SECTION("repeat")
                {
                    SECTION("Count")
                    {
                        float inputs[] = { 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 2;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(count:Num):Num = List.repeat(1, count).count;";
                        SECTION("Count clamps to 1 if less than 1")
                        {
                            inputs[0] = -100;
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 1);
                        }

                        SECTION("Count can be the max of a float")
                        {
#undef max //windows please
                            inputs[0] = std::numeric_limits<float>::max();
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == std::numeric_limits<float>::max());
                        }

                        SECTION("Count is NaN, set to 1")
                        {
                            inputs[0] = std::nanf("");
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 1);
                        }

                        SECTION("Count is PositiveInfinity, set to 1")
                        {
                            inputs[0] = std::numeric_limits<float>::infinity();
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 1);
                        }

                        SECTION("Count is NegativeInfinity, set to 1")
                        {
                            inputs[0] = std::numeric_limits<float>::infinity() * -1.0f;
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 1);
                        }
                    }

                    SECTION("List.repeat(3, count).at(idx)")
                    {
                        float inputs[] = { 3, 2 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 2;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(count:Num, idx:Num):Num = List.repeat(3, count).at(idx);";

                        SECTION("Valid Count, Non-Integer")
                        {
                            inputs[0] = 100.5f;
                            SECTION("Count truncates to integer")
                            {
                                result = eval_with_inputs("evaluate(count:Num):Num = List.repeat(3, count).count;", &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 100.0f);
                            }

                            SECTION("Negative Index")
                            {
                                inputs[1] = -200;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 3);
                            }

                            SECTION("Valid Index")
                            {
                                for (int i = 0; i < static_cast<int>(inputs[0]); ++i)
                                {
                                    inputs[1] = static_cast<float>(i);
                                    result = eval_with_inputs(src, &input, &output);
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == 3);
                                }
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[1] = inputs[0] + 2.0f;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 3);
                            }
                        }
                    }

                    SECTION("List.repeat(1, 0).at(idx)")
                    {
                        float inputs[] = { 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 2;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(count:Num, idx:Num):Num = List.repeat(1, count).at(idx);";

                        SECTION("Zero Count")
                        {
                            SECTION("Negative Index")
                            {
                                inputs[1] = -1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("Valid Index")
                            {
                                inputs[1] = 0;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[1] = 1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }
                        }
                    }

                    SECTION("List.repeat(1, -100).at(idx)")
                    {
                        float inputs[] = { 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 2;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(count:Num, idx:Num):Num = List.repeat(1, count).at(idx);";

                        SECTION("Negative Count")
                        {
                            inputs[0] = -100.0f;
                            SECTION("Negative Index")
                            {
                                inputs[1] = -200;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("Valid Index")
                            {
                                inputs[1] = 0;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[1] = 1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }
                        }
                    }

                    SECTION("List.repeat(10, -10).concatenate(List.repeat(20, -10))")
                    {
                        float inputs[] = { -10, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 2;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        std::string src = "evaluate(count:Num, idx:Num):Num = List.repeat(10, count).concatenate(List.repeat(20, count))";

                        SECTION("Resulting list has a count of 2")
                        {
                            auto new_src = fmt::format("{}.count;", src);
                            result = eval_with_inputs(new_src.c_str(), &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 2);
                        }

                        SECTION("Resulting list indexed at 0 is 10")
                        {
                            inputs[1] = 0;
                            auto new_src = fmt::format("{}.at(idx);", src);
                            result = eval_with_inputs(new_src.c_str(), &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 10);
                        }

                        SECTION("Resulting list indexed at 1 is 20")
                        {
                            inputs[1] = 1;
                            auto new_src = fmt::format("{}.at(idx);", src);
                            result = eval_with_inputs(new_src.c_str(), &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 20);
                        }
                    }
                }

                SECTION("range")
                {
                    SECTION("List.range(4, 6).at(1)")
                    {
                        float inputs[] = { 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 1;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(idx:Num):Num = List.range(4, 6).at(1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 5);
                    }

                    SECTION("List.range(4, 6).at(idx)")
                    {
                        float inputs[] = { 6 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 1;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(idx:Num):Num = List.range(4, 6).at(idx);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 9);
                    }

                    SECTION("List.range(4, 6).at(-1)")
                    {
                        float inputs[] = { 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 1;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(idx:Num):Num = List.range(4, 6).at(-1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 4);
                    }

                    SECTION("List.range(4, 6).at(100)")
                    {
                        float inputs[] = { 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 1;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(idx:Num):Num = List.range(4, 6).at(100);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 9);
                    }
                }

                SECTION("concatenate")
                {
                    SECTION("list(a, b, c).concatenate(list(4, 5, 6)).at(5)")
                    {
                        float inputs[] = { 1, 2, 3 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(a, b, c).concatenate(list(4, 5, 6)).at(5);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 6);
                    }
                }

                SECTION("take")
                {
                    SECTION("list(1, 2, 3).take(-1).at(1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).take(-1).at(1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 1);
                    }

                    SECTION("list(1, 2, 3).take(-1).at(-1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).take(-1).at(-1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 1);
                    }

                    SECTION("list(1, 2, 3).take(-1).at(4)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).take(-1).at(4);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 1);
                    }

                    SECTION("list(1, 2, 3).take(1).at(1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).take(1).at(1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 1);
                    }

                    SECTION("list(1, 2, 3).take(1).at(-1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).take(1).at(-1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 1);
                    }

                    SECTION("list(1, 2, 3).take(1).at(4)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).take(1).at(4);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 1);
                    }

                    SECTION("list(1, 2, 3).take(4).at(1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).take(4).at(1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 2);
                    }

                    SECTION("list(1, 2, 3).take(4).at(-1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).take(4).at(-1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 1);
                    }

                    SECTION("list(1, 2, 3).take(4).at(4)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).take(4).at(4);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);
                    }

                    SECTION("list(1, 2, 3).take(2).at(2)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).take(2).at(2);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 2);
                    }
                }

                SECTION("skip")
                {
                    SECTION("list(1, 2, 3).skip(-4).at(1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).skip(-4).at(1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 2);
                    }

                    SECTION("list(1, 2, 3).skip(-4).at(-1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).skip(-4).at(-1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 1);
                    }

                    SECTION("list(1, 2, 3).skip(-4).at(4)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).skip(-4).at(4);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);
                    }

                    SECTION("list(1, 2, 3).skip(-1).at(1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).skip(-1).at(1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 2);
                    }

                    SECTION("list(1, 2, 3).skip(-1).at(-1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).skip(-1).at(-1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 1);
                    }

                    SECTION("list(1, 2, 3).skip(-1).at(4)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).skip(-1).at(4);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);
                    }

                    SECTION("list(1, 2, 3).skip(1).at(1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).skip(1).at(1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);
                    }

                    SECTION("list(1, 2, 3).skip(1).at(-1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).skip(1).at(-1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 2);
                    }

                    SECTION("list(1, 2, 3).skip(1).at(4)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).skip(1).at(4);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);
                    }

                    SECTION("list(1, 2, 3).skip(4).at(1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).skip(4).at(1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);
                    }

                    SECTION("list(1, 2, 3).skip(4).at(-1)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).skip(4).at(-1);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);
                    }

                    SECTION("list(1, 2, 3).skip(4).at(4)")
                    {
                        float inputs[] = { 0, 0, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 3;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num, b:Num, c:Num):Num = list(1, 2, 3).skip(4).at(4);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);
                    }
                }

                SECTION("slice")
                {
                    SECTION("list(1, 2, 3).slice(1, 1).at(idx)")
                    {
                        float inputs[] = { 1, 2, 3, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 4;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = list(a, b, c).slice(1, 1).at(idx);";

                        SECTION("Runtime Expression List, Runtime Index")
                        {
                            SECTION("Negative Index")
                            {
                                inputs[3] = -1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 2);
                            }

                            SECTION("Valid Index")
                            {
                                inputs[3] = 0;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 2);
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[3] = 1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 2);
                            }

                            SECTION("PositiveInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity();
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 2);
                            }

                            SECTION("NegativeInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity() * -1.0f;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 2);
                            }

                            SECTION("NaN Index")
                            {
                                inputs[3] = std::nanf("");
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 2);
                            }
                        }
                    }

                    SECTION("list(1, 2, 3).slice(-1, -1).at(idx)")
                    {
                        float inputs[] = { 1, 2, 3, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 4;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = list(a, b, c).slice(-1, -1).at(idx);";

                        SECTION("Runtime Expression List, Runtime Index")
                        {
                            SECTION("Negative Index")
                            {
                                inputs[3] = -1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("Valid Index")
                            {
                                inputs[3] = 0;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[3] = 1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("PositiveInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity();
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("NegativeInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity() * -1.0f;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("NaN Index")
                            {
                                inputs[3] = std::nanf("");
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }
                        }
                    }

                    SECTION("list(1, 2, 3).slice(4, -1).at(idx)")
                    {
                        float inputs[] = { 1, 2, 3, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 4;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = list(a, b, c).slice(4, -1).at(idx);";

                        SECTION("Runtime Expression List, Runtime Index")
                        {
                            SECTION("Negative Index")
                            {
                                inputs[3] = -1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("Valid Index")
                            {
                                inputs[3] = 0;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[3] = 1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("PositiveInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity();
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("NegativeInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity() * -1.0f;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("NaN Index")
                            {
                                inputs[3] = std::nanf("");
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }
                        }
                    }

                    SECTION("list(1, 2, 3).slice(-1, 4).at(idx)")
                    {
                        float inputs[] = { 1, 2, 3, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 4;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = list(a, b, c).slice(-1, 4).at(idx);";

                        SECTION("Runtime Expression List, Runtime Index")
                        {
                            SECTION("Negative Index")
                            {
                                inputs[3] = -1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[0]);
                            }

                            SECTION("Valid Index")
                            {
                                for (int i = 0; i < 3; ++i)
                                {
                                    inputs[3] = i;
                                    result = eval_with_inputs(src, &input, &output);
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == inputs[i]);
                                }
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[3] = 3;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("PositiveInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity();
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("NegativeInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity() * -1.0f;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[0]);
                            }

                            SECTION("NaN Index")
                            {
                                inputs[3] = std::nanf("");
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[0]);
                            }
                        }
                    }

                    SECTION("list(1, 2, 3).slice(2, 4).at(idx)")
                    {
                        float inputs[] = { 1, 2, 3, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 4;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = list(a, b, c).slice(2, 4).at(idx);";

                        SECTION("Runtime Expression List, Runtime Index")
                        {
                            SECTION("Negative Index")
                            {
                                inputs[3] = -1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("Valid Index")
                            {
                                inputs[3] = 0;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[3] = 1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("PositiveInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity();
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("NegativeInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity() * -1.0f;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("NaN Index")
                            {
                                inputs[3] = std::nanf("");
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }
                        }
                    }

                    SECTION("list(1, 2, 3).slice(4, 2).at(idx)")
                    {
                        float inputs[] = { 1, 2, 3, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 4;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = list(a, b, c).slice(4, 2).at(idx);";

                        SECTION("Runtime Expression List, Runtime Index")
                        {
                            SECTION("Negative Index")
                            {
                                inputs[3] = -1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("Valid Index")
                            {
                                inputs[3] = 0;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[3] = 1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("PositiveInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity();
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("NegativeInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity() * -1.0f;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("NaN Index")
                            {
                                inputs[3] = std::nanf("");
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }
                        }
                    }

                    SECTION("list(1, 2, 3).slice(2, -4).at(idx)")
                    {
                        float inputs[] = { 1, 2, 3, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 4;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = list(a, b, c).slice(2, -4).at(idx);";

                        SECTION("Runtime Expression List, Runtime Index")
                        {
                            SECTION("Negative Index")
                            {
                                inputs[3] = -1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("Valid Index")
                            {
                                inputs[3] = 0;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[3] = 1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("PositiveInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity();
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("NegativeInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity() * -1.0f;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }

                            SECTION("NaN Index")
                            {
                                inputs[3] = std::nanf("");
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[2]);
                            }
                        }
                    }

                    SECTION("list(1, 2, 3).slice(-4, 2).at(idx)")
                    {
                        float inputs[] = { 1, 2, 3, 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 4;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(a:Num, b:Num, c:Num, idx:Num):Num = list(a, b, c).slice(-4, 2).at(idx);";

                        SECTION("Runtime Expression List, Runtime Index")
                        {
                            SECTION("Negative Index")
                            {
                                inputs[3] = -1;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[0]);
                            }

                            SECTION("Valid Index")
                            {
                                for (int i = 0; i < 2; ++i)
                                {
                                    inputs[3] = i;
                                    result = eval_with_inputs(src, &input, &output);
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == inputs[i]);
                                }
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[3] = 2;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[1]);
                            }

                            SECTION("PositiveInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity();
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[1]);
                            }

                            SECTION("NegativeInfinity Index")
                            {
                                inputs[3] = std::numeric_limits<float>::infinity() * -1.0f;
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[0]);
                            }

                            SECTION("NaN Index")
                            {
                                inputs[3] = std::nanf("");
                                result = eval_with_inputs(src, &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == inputs[0]);
                            }
                        }
                    }

                    SECTION("list(1, 2, 3).slice(2, 2).at(a)")
                    {
                        float inputs[] = { 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 1;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3).slice(2, 2).at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);

                        inputs[0] = 1;
                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3).slice(2, 2).at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);

                        inputs[0] = -1;
                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3).slice(2, 2).at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);
                    }

                    SECTION("list(1, 2, 3, 4).slice(2, 3).at(a)")
                    {
                        float inputs[] = { 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 1;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3, 4).slice(2, 3).at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);

                        inputs[0] = 1;
                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3, 4).slice(2, 3).at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 4);

                        inputs[0] = 2;
                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3, 4).slice(2, 3).at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 4);

                        inputs[0] = -1;
                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3, 4).slice(2, 3).at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);
                    }

                    SECTION("list(1, 2, 3, 4).slice(3, 2).at(a)")
                    {
                        float inputs[] = { 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 1;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3, 4).slice(3, 2).at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 4);

                        inputs[0] = 1;
                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3, 4).slice(3, 2).at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 4);

                        inputs[0] = 2;
                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3, 4).slice(3, 2).at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 4);

                        inputs[0] = -1;
                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3, 4).slice(3, 2).at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 4);
                    }
                }

                SECTION("cycle")
                {
                    SECTION("list(1, 2, 3).cycle.at(a)")
                    {
                        float inputs[] = { 3 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 1;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3).cycle.at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 1);

                        inputs[0] = 4;
                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3).cycle.at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 2);

                        inputs[0] = 5;
                        result = eval_with_inputs("evaluate(a:Num):Num = list(1, 2, 3).cycle.at(a);", &input, &output);
                        REQUIRE(result == ELEMENT_OK);
                        REQUIRE(outputs[0] == 3);
                    }
                }

                SECTION("reverse")
                {
                    SECTION("list(1, 2, 3).reverse.at(a)")
                    {
                        float inputs[] = { 0 };
                        element_inputs input;
                        input.values = inputs;
                        input.count = 1;
                        element_outputs output;
                        float outputs[] = { 0 };
                        output.values = outputs;
                        output.count = 1;

                        const char* src = "evaluate(a:Num):Num = list(1, 2, 3).reverse.at({});";

                        SECTION("Constant List, Dynamic Index")
                        {
                            auto new_src = fmt::format(src, "a");
                            SECTION("Negative Index")
                            {
                                inputs[0] = -1;
                                result = eval_with_inputs(new_src.c_str(), &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 3);
                            }

                            SECTION("Valid Index")
                            {
                                for (int i = 0; i < 3; ++i)
                                {
                                    inputs[0] = i;
                                    result = eval_with_inputs(new_src.c_str(), &input, &output);
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == 3 - i);
                                }
                            }

                            SECTION("Beyond Length Index")
                            {
                                inputs[0] = 3;
                                result = eval_with_inputs(new_src.c_str(), &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("NegativeInfinity Index")
                            {
                                inputs[0] = std::numeric_limits<float>::infinity() * -1.0f;
                                result = eval_with_inputs(new_src.c_str(), &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 3);
                            }

                            SECTION("PositiveInfinity Index")
                            {
                                inputs[0] = std::numeric_limits<float>::infinity();
                                result = eval_with_inputs(new_src.c_str(), &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("NaN Index")
                            {
                                inputs[0] = std::nanf("");
                                result = eval_with_inputs(new_src.c_str(), &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 3);
                            }
                        }

                        SECTION("Constant List, Constant Index")
                        {
                            SECTION("Negative Index")
                            {
                                auto new_src = fmt::format(src, -1);
                                result = eval_with_inputs(new_src.c_str(), &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 3);
                            }

                            SECTION("Valid Index")
                            {
                                for (int i = 0; i < 3; ++i)
                                {
                                    auto new_src = fmt::format(src, i);
                                    result = eval_with_inputs(new_src.c_str(), &input, &output);
                                    REQUIRE(result == ELEMENT_OK);
                                    REQUIRE(outputs[0] == 3 - i);
                                }
                            }

                            SECTION("Beyond Length Index")
                            {
                                auto new_src = fmt::format(src, 3);
                                result = eval_with_inputs(new_src.c_str(), &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("NegativeInfinity Index")
                            {
                                auto new_src = fmt::format(src, "Num.NegativeInfinity");
                                result = eval_with_inputs(new_src.c_str(), &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 3);
                            }

                            SECTION("PositiveInfinity Index")
                            {
                                auto new_src = fmt::format(src, "Num.PositiveInfinity");
                                result = eval_with_inputs(new_src.c_str(), &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 1);
                            }

                            SECTION("NaN Index")
                            {
                                auto new_src = fmt::format(src, "Num.NaN");
                                result = eval_with_inputs(new_src.c_str(), &input, &output);
                                REQUIRE(result == ELEMENT_OK);
                                REQUIRE(outputs[0] == 3);
                            }
                        }
                    }
                }

                SECTION("filter")
                {
                    SECTION("list(1, 2, 3).filter")
                    {
                        //todo: requires countWhere, which requires fold
                        /*
                        filter(a:List, predicate:Predicate):List
                        {
                            count = countWhere(a, predicate);
                            index(idx:Num) = idx.add(a.slice(0, idx).countWhere(_(item) = predicate(item).negate));
                            return = List(index, count);
                        }
                        */
                        REQUIRE(1 == 0);
                    }
                }

                SECTION("countWhere")
                {
                    SECTION("list(1, 2, 3).countWhere")
                    {

                        //todo: requires fold
                        //countWhere(a:List, predicate : Predicate) :Num = a.fold(0, _(current, next) = if (predicate(next), add(current, 1), current));
                        REQUIRE(1 == 0);
                    }
                }

                SECTION("findLast")
                {
                    SECTION("list(1, 2, 3).findLast")
                    {
                        //todo: requires fold
                        //findLast(a:List, predicate:Predicate, default) = a.fold(default, _(current, next) = predicate(next).if(next, current));
                        REQUIRE(1 == 0);
                    }
                }

                SECTION("findFirst")
                {
                    SECTION("list(1, 2, 3).findFirst")
                    {
                        //todo: requires fold
                        //findFirst(a:List, predicate:Predicate, default) = a.reverse.findLast(predicate, default);
                        REQUIRE(1 == 0);
                    }
                }

                SECTION("fold")
                {
                    SECTION("list(1, 2, 3).fold(start, Num.add)")
                    {
                        std::array<float, 4> inputs = { 1, 2, 3, 0 };
                        std::array<float, 1> outputs = { 0 };
                        element_inputs input{ inputs.data(), inputs.size() };
                        element_outputs output{ outputs.data(), outputs.size() };

                        const char* src = "evaluate(a:Num, b:Num, c:Num, start:Num):Num = list(a, b, c).fold(start, Num.add);";

                        SECTION("Negative Start")
                        {
                            inputs[3] = -100;
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[3] + inputs[0] + inputs[1] + inputs[2]);
                        }

                        SECTION("Zero Start")
                        {
                            inputs[3] = 0;
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[3] + inputs[0] + inputs[1] + inputs[2]);
                        }

                        SECTION("Positive Start")
                        {
                            inputs[3] = 100;
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[3] + inputs[0] + inputs[1] + inputs[2]);
                        }

                        SECTION("Max Value Start")
                        {
                            inputs[3] = std::numeric_limits<float>::max();
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[3] + inputs[0] + inputs[1] + inputs[2]);
                        }

                        SECTION("NaN Start")
                        {
                            inputs[3] = std::nanf("");
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[3] + inputs[0] + inputs[1] + inputs[2]);
                        }

                        SECTION("PositiveInfinity Start")
                        {
                            inputs[3] = std::numeric_limits<float>::infinity();
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[3] + inputs[0] + inputs[1] + inputs[2]);
                        }

                        SECTION("NegativeInfinity Start")
                        {
                            inputs[3] = std::numeric_limits<float>::infinity() * -1.0f;
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[3] + inputs[0] + inputs[1] + inputs[2]);
                        }
                    }

                    SECTION("list(False, False, False).fold(start, Bool.or)")
                    {
                        std::array<float, 4> inputs = { 0, 0, 0, 0 };
                        std::array<float, 1> outputs = { 0 };
                        element_inputs input{ inputs.data(), inputs.size() };
                        element_outputs output{ outputs.data(), outputs.size() };

                        const char* src = "evaluate(a:Bool, b:Bool, c:Bool, start:Bool):Bool = list(a, b, c).fold(start, Bool.or);";

                        SECTION("Negative Start")
                        {
                            inputs[3] = -100;
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 0);
                        }

                        SECTION("Zero Start")
                        {
                            inputs[3] = 0;
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 0);
                        }

                        SECTION("Positive Start")
                        {
                            inputs[3] = 100;
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 1);
                        }

                        SECTION("Max Value Start")
                        {
                            inputs[3] = std::numeric_limits<float>::max();
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 1);
                        }

                        SECTION("NaN Start")
                        {
                            inputs[3] = std::nanf("");
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == static_cast<float>(static_cast<bool>(inputs[3]) || false));
                        }

                        SECTION("PositiveInfinity Start")
                        {
                            inputs[3] = std::numeric_limits<float>::infinity();
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 1);
                        }

                        SECTION("NegativeInfinity Start")
                        {
                            inputs[3] = std::numeric_limits<float>::infinity() * -1.0f;
                            result = eval_with_inputs(src, &input, &output);
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == 0);
                        }
                    }

                    SECTION("list(Vector2, Vector2, Vector2).fold(Vector2(start, start), Vector2.mul).x")
                    {
                        std::array<float, 7> inputs = { 2, 2, 2, 2, 2, 2, 0 };
                        std::array<float, 2> outputs = { 0 };
                        element_inputs input{ inputs.data(), inputs.size() };
                        element_outputs output{ outputs.data(), outputs.size() };

                        const char* src = "evaluate(a:Vector2, b:Vector2, c:Vector2, start:Num):Num = list(a, b, c).fold(Vector2(start, start), Vector2.mul).x;";

                        SECTION("Negative Start")
                        {
                            inputs[6] = -100;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[6] * inputs[0] * inputs[2] * inputs[4]);
                        }

                        SECTION("Zero Start")
                        {
                            inputs[6] = 0;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[6] * inputs[0] * inputs[2] * inputs[4]);
                        }

                        SECTION("Positive Start")
                        {
                            inputs[6] = 100;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[6] * inputs[0] * inputs[2] * inputs[4]);
                        }

                        SECTION("Max Value Start")
                        {
                            inputs[6] = std::numeric_limits<float>::max();
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[6] * inputs[0] * inputs[2] * inputs[4]);
                        }

                        SECTION("NaN Start")
                        {
                            inputs[6] = std::nanf("");
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[6] * inputs[0] * inputs[2] * inputs[4]);
                        }

                        SECTION("PositiveInfinity Start")
                        {
                            inputs[6] = std::numeric_limits<float>::infinity();
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[6] * inputs[0] * inputs[2] * inputs[4]);
                        }

                        SECTION("NegativeInfinity Start")
                        {
                            inputs[6] = std::numeric_limits<float>::infinity() * -1.0f;
                            result = eval_with_inputs(src, &input, &output, "StandardLibrary");
                            REQUIRE(result == ELEMENT_OK);
                            REQUIRE(outputs[0] == inputs[6] * inputs[0] * inputs[2] * inputs[4]);
                        }
                    }
                }
            }

            SECTION("for")
            {
                SECTION("Compile-time for")
                {
                    float inputs[] = { -1 };
                    element_inputs input;
                    input.values = inputs;
                    input.count = 1;
                    element_outputs output;
                    float outputs[] = { 0 };
                    output.values = outputs;
                    output.count = 1;

                    char source[] = "struct test(value:Num); evaluate(a:Num):Num = for(test(0), _(b):Bool = b.value.lt(4), _(c) = test(c.value.add(1))).value;";
                    result = eval_with_inputs(source, &input, &output);

                    REQUIRE(result == ELEMENT_OK);
                    REQUIRE(outputs[0] == 4);
                }

                SECTION("Dynamic-time for, initial")
                {
                    float inputs[] = { 0 };
                    element_inputs input;
                    input.values = inputs;
                    input.count = 1;
                    element_outputs output;
                    float outputs[] = { 0 };
                    output.values = outputs;
                    output.count = 1;

                    char source[] = "struct test(value:Num); evaluate(a:Num):Num = for(test(a), _(b:test):Bool = b.value.lt(4), _(c:test):test = test(c.value.add(1))).value;";
                    result = eval_with_inputs(source, &input, &output);

                    REQUIRE(result == ELEMENT_OK);
                    REQUIRE(outputs[0] == 4);
                }

                SECTION("Dynamic-time for, predicate")
                {
                    float inputs[] = { 4 };
                    element_inputs input;
                    input.values = inputs;
                    input.count = 1;
                    element_outputs output;
                    float outputs[] = { 0 };
                    output.values = outputs;
                    output.count = 1;

                    char source[] = "struct test(value:Num); evaluate(a:Num):Num = for(test(0), _(b:test):Bool = b.value.lt(a), _(c:test):test = test(c.value.add(1))).value;";
                    result = eval_with_inputs(source, &input, &output);

                    REQUIRE(result == ELEMENT_OK);
                    REQUIRE(outputs[0] == 4);
                }

                SECTION("Dynamic-time for, body")
                {
                    float inputs[] = { 1 };
                    element_inputs input;
                    input.values = inputs;
                    input.count = 1;
                    element_outputs output;
                    float outputs[] = { 0 };
                    output.values = outputs;
                    output.count = 1;

                    char source[] = "struct test(value:Num); evaluate(a:Num):Num = for(test(0), _(b:test):Bool = b.value.lt(4), _(c:test):test = test(c.value.add(a))).value;";
                    result = eval_with_inputs(source, &input, &output);

                    REQUIRE(result == ELEMENT_OK);
                    REQUIRE(outputs[0] == 4);
                }

                SECTION("Dynamic-time for, nested")
                {
                    float inputs[] = { 0 };
                    element_inputs input;
                    input.values = inputs;
                    input.count = 1;
                    element_outputs output;
                    float outputs[] = { 0 };
                    output.values = outputs;
                    output.count = 1;

                    char source[] = ""
                        "struct test(value:Num);\n"
                        "evaluate(a:Num):Num\n"
                        "{\n"
                        "   nested_predicate(input:Num):Bool = input.lt(20);\n"
                        "   nested_body(input:Num):Num = input.add(1);\n"
                        "   body(input:test):test = test(input.value.add(for(a, nested_predicate, nested_body)));\n"
                        "   predicate(input:test):Bool = input.value.lt(200);\n"
                        "   return = for(test(a), predicate, body).value;\n"
                        "}\n";
                    result = eval_with_inputs(source, &input, &output);

                    REQUIRE(result == ELEMENT_OK);
                    REQUIRE(outputs[0] == 200);
                }
            }
        }

        SECTION("StandardLibrary")
        {
            SECTION("Vector2")
            {
                SECTION("opposite")
                {
                    float inputs[] = { 1, 2 };
                    element_inputs input;
                    input.values = inputs;
                    input.count = 2;
                    element_outputs output;
                    float outputs[] = { 0, 0 };
                    output.values = outputs;
                    output.count = 2;

                    char source[] = "evaluate(a:Num):Num = Bool.if(Bool(a), a, a.mul(2));";

                    result = eval_with_inputs("evaluate(a:Vector2):Vector2 = a.opposite;", &input, &output, "StandardLibrary");

                    REQUIRE(result == ELEMENT_OK);
                    REQUIRE(output.values[0] == -input.values[0]);
                    REQUIRE(output.values[1] == -input.values[1]);
                }
            }
        }
    }

    SECTION("Compile time evaluation")
    {
        element_result result = ELEMENT_OK;

        SECTION("Expression bodied function. Literal.") {
            result = eval("evaluate = -3;");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Intrinsic struct. intrinsic function. calling with arguments") {
            result = eval("evaluate = Num.add(1, 2);");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Instance function") {
            result = eval("evaluate = 1.add(2);");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Intrinsic nullary") {
            result = eval("evaluate = Num.NaN;");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Nullary") {
            result = eval("evaluate = Num.pi;");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Nullary which looks up another nullary locally") {
            result = eval("evaluate = Num.tau;");
            REQUIRE(result == ELEMENT_OK);
        }

        //element doesn't support partial application of any function, only instance functions (i.e. member functions with implicit "this")
        /*
        SECTION("Partial application") {
            result = eval("add5 = Num.add(5); evaluate = add5(2);");
            REQUIRE(result == ELEMENT_OK);
        }
         */

        SECTION("Indexing intrinsic function") {
            result = eval("evaluate = Num.acos(0).degrees;");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Indexing nested instance function") {
            result = eval("evaluate = Num.cos(Num.pi.div(4));");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Chaining Functions") {
            //ndexing degrees on result of asin. instance function degrees that is now nullary.
            result = eval("evaluate = Num.asin(-1).degrees;");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("If Expression") {
            result = eval("evaluate = Bool.if(False, 1, 0);");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Mod") {
            result = eval("evaluate = Num.mod(5, 2);");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Bool constructor") {
            result = eval("evaluate = Bool(2);");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Num constructor converts back to num") {
            result = eval("evaluate = Num(Bool(Num(2))).mul(1);");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Pass higher order function") {
            result = eval("double(a:Num) = a.mul(2); applyFive(a) = a(5); evaluate = applyFive(double);");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Functions") {
            result = eval("mul(a) { return(b) = a.mul(b); } evaluate = mul(5)(2);");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Struct fields") {
            result = eval("struct MyStruct(a:Num, b:Num) {} evaluate = MyStruct(1, 2).a;");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Struct fields") {
            result = eval("struct MyStruct(a:Num, b:Num) {} evaluate = MyStruct(1, 2).b;");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Struct instance function") {
            result = eval("struct MyStruct(a:Num, b:Num) {add(s:MyStruct) = s.a.add(s.b);} evaluate = MyStruct(1, 2).add;");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Struct instance function called with more parameters") {
            result = eval("struct MyStruct(a:Num, b:Num) {add(s:MyStruct, s2:MyStruct) = MyStruct(s.a.add(s2.a), s.b.add(s2.b));} evaluate = MyStruct(1, 2).add(MyStruct(1, 2)).b;");
            REQUIRE(result == ELEMENT_OK);
        }

        SECTION("Typename can refer to things inside scopes") {
            result = eval("namespace Space { struct Thing(a){}} func(a:Space.Thing) = a.a; evaluate = func(Space.Thing(1));");
        }

        SECTION("Error - Circular Compilation")
        {
            result = eval(
                "c(d) = a(d);\n"
                "a(b:Num) = c(b.mul(b));\n"
                "evaluate = a(5);\n");
            REQUIRE(result == ELEMENT_ERROR_CIRCULAR_COMPILATION);
        }

        SECTION("Error - Identifier Not Found")
        {
            result = eval("evaluate = one;");
            REQUIRE(result == ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);
        }

        SECTION("Error - Cannot Be Used As Instance Function")
        {
            //todo: missing a semi colon is an unfriendly error
            result = eval("struct MyStruct(crap) { f(a:Num) = a; } evaluate = MyStruct(1).f;");
            REQUIRE(result == ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION);
        }

        SECTION("Error - Identifier Not Found - Indexing Struct")
        {
            result = eval("struct MyStruct(crap) { f(a:Num) = a; } evaluate = MyStruct(1).a;");
            REQUIRE(result == ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);
        }

        SECTION("Error - Identifier Not Found - Indexing Namespace")
        {
            result = eval("namespace MyNamespace {} evaluate = MyNamespace.a;");
            REQUIRE(result == ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);
        }

        SECTION("Error - Missing Contents For Call")
        {
            result = eval("namespace MyNamespace {} evaluate = MyNamespace();");
            REQUIRE(result == ELEMENT_ERROR_MISSING_CONTENTS_FOR_CALL);
        }

        SECTION("Error - Calling Namespace")
        {
            //todo: namespace should have a specific error message for calling a namespace, with a specific error code
            result = eval("namespace MyNamespace {} evaluate = MyNamespace(1);");
            REQUIRE(result == ELEMENT_ERROR_UNKNOWN);
        }

        SECTION("Error - Indexing Function")
        {
            //todo: this should return an error for trying to index a function
            result = eval("func(a) = 1; evaluate = func.a;");
            REQUIRE(result == ELEMENT_ERROR_UNKNOWN);
        }

        SECTION("Error - Partial Grammar")
        {
            result = eval("evaluate\n");
            REQUIRE(result == ELEMENT_ERROR_PARTIAL_GRAMMAR);
        }

        SECTION("Error - Missing Function Body")
        {
            result = eval("evaluate;\n");
            REQUIRE(result == ELEMENT_ERROR_MISSING_FUNCTION_BODY);
        }

        SECTION("Error - Invalid Identifier - Semicolon")
        {
            result = eval(";");
            REQUIRE(result == ELEMENT_ERROR_INVALID_IDENTIFIER);
        }

        SECTION("Error - Invalid Expression In Call - Semicolon")
        {
            result = eval("func = 1; evaluate = func(;);");
            REQUIRE(result == ELEMENT_ERROR_INVALID_EXPRESSION);
        }

        SECTION("Error - Invalid Identifier When Indexing - Semicolon")
        {
            result = eval("func = 1; evaluate = func(a.;);");
            REQUIRE(result == ELEMENT_ERROR_INVALID_IDENTIFIER);
        }

        SECTION("Error - Missing Parenthesis For Call")
        {
            result = eval("func = 1; evaluate = func(a;);");
            REQUIRE(result == ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL);
        }

        SECTION("Error - Invalid Expression In Call - (a,,)")
        {
            result = eval("func = 1; evaluate = func(a,,);");
            REQUIRE(result == ELEMENT_ERROR_INVALID_EXPRESSION);
        }

        SECTION("Error - Invalid Expression In Call - (a,)")
        {
            result = eval("func = 1; evaluate = func(a,);");
            REQUIRE(result == ELEMENT_ERROR_INVALID_EXPRESSION);
        }

        SECTION("Error - Invalid Expression In Call - (,)")
        {
            result = eval("func = 1; evaluate = func(,);");
            REQUIRE(result == ELEMENT_ERROR_INVALID_EXPRESSION);
        }

        SECTION("Error - Invalid Port")
        {
            result = eval("func(,) = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_PORT);
        }

        SECTION("Error - Invalid Typename")
        {
            result = eval("func(a:,) = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_TYPENAME);
        }

        SECTION("Error - Invalid Typename")
        {
            result = eval("func(a::) = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_TYPENAME);
        }

        SECTION("Error - Invalid Typename")
        {
            result = eval("func(a:) = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_TYPENAME);
        }

        SECTION("Error - Invalid Typename")
        {
            result = eval("func(a: = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_TYPENAME);
        }

        SECTION("Error - Missing Closing Parenthesis For Portlist")
        {
            //note: expressions aren't valid in a typename, might change?
            result = eval("func(a:func(a)) = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_MISSING_CLOSING_PARENTHESIS_FOR_PORTLIST);
        }

        SECTION("Error - Invalid Typename - 123")
        {
            result = eval("func(a:123) = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_TYPENAME);
        }

        SECTION("Error - Invalid Identifier - 123")
        {
            result = eval("123 = 1;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_IDENTIFIER);
        }

        SECTION("Error - Identifier Not Found")
        {
            //empty is okay, but we look for "evaluate", which produces an error
            result = eval("");
            REQUIRE(result == ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);
        }

        SECTION("Error - Argument Count Mismatch")
        {
            //should log two errors, though we'll only be told the error code for one of them in our API
            result = eval("evaluate = Num.add(1).add(Num.add(1, 2, 3));");
            REQUIRE(result == ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH);
        }

        SECTION("Error - Indexing Arity Function")
        {
            //todo: better error message
            result = eval("evaluate = 5.add.add(1);");
            REQUIRE(result == ELEMENT_ERROR_UNKNOWN);
        }

        SECTION("Error - Identifier Not Found - Indexing Num")
        {
            //todo: better error message
            result = eval("evaluate = Num.woo;");
            REQUIRE(result == ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);
        }

        SECTION("Error - Identifier Not Found - Indexing 5")
        {
            //todo: better error message
            result = eval("evaluate = 5.woo;");
            REQUIRE(result == ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);
        }

        {
            char my_vec[] =
                "struct MyVec(x:Num, y:Num)\n"
                "{\n"
                "    add(a:MyVec, b:MyVec) = MyVec(a.x.add(b.x), a.y.add(b.y));\n"
                "    transform_components(a:MyVec, func) = MyVec(func(a.x), func(a.y), func(a.z));\n"
                "}\n";

            SECTION("Error - Missing Closing Parenthesis For Call")
            {
                result = eval_with_source(my_vec, "evaluate = MyStruct(1, 2, 3).add(MyStruct(4, 5, 6);\n");
                REQUIRE(result == ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL);
            }

            SECTION("Error - Missing Semicolon")
            {
                result = eval_with_source(my_vec, "evaluate = MyStruct(1, 2, 3).add(MyStruct4, 5, 6));\n");
                REQUIRE(result == ELEMENT_ERROR_MISSING_SEMICOLON);
            }

            SECTION("Error - Missing Closing Parenthesis For Call #2")
            {
                //todo: we should say what the opening bracket was/specifically which "MyStruct" it is referring to
                result = eval_with_source(my_vec, "evaluate = MyStruct(1, 2, 3.add(MyStruct(4, 5, 6));\n");
                REQUIRE(result == ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL);
            }

            SECTION("Error - Missing Function Body")
            {
                //todo: we should print that we expect '=' or '{' when defining the function 'evaluate' but found 'MyStruct' instead
                result = eval_with_source(my_vec, "evaluate MyStruct(1, 2, 3).add(MyStruct(4, 5, 6));\n");
                REQUIRE(result == ELEMENT_ERROR_MISSING_FUNCTION_BODY);
            }
        }
    }

    SECTION("element_interpreter_evaluate_expression once")
    {
        element_interpreter_ctx* context;
        element_interpreter_create(&context);

        element_outputs output;
        float outputs_buffer[] = { 0 };
        output.values = outputs_buffer;
        output.count = 1;

        auto result = element_interpreter_evaluate_expression(context, nullptr, "1", &output);
        REQUIRE(result == ELEMENT_OK);
        REQUIRE(outputs_buffer[0] == 1);
        element_interpreter_delete(context);
    }

    SECTION("element_interpreter_evaluate_expression twice")
    {
        element_interpreter_ctx* context;
        element_interpreter_create(&context);

        element_outputs output;
        float outputs_buffer[] = { 0 };
        output.values = outputs_buffer;
        output.count = 1;

        auto result = element_interpreter_evaluate_expression(context, nullptr, "1", &output);
        REQUIRE(result == ELEMENT_OK);
        REQUIRE(outputs_buffer[0] == 1);

        result = element_interpreter_evaluate_expression(context, nullptr, "2", &output);
        REQUIRE(result == ELEMENT_OK);
        REQUIRE(outputs_buffer[0] == 2);
        element_interpreter_delete(context);
    }

    SECTION("element_interpreter_typeof_expression once")
    {
        element_interpreter_ctx* context;
        element_interpreter_create(&context);

        std::string buffer(256, '\0');

        auto result = element_interpreter_typeof_expression(context, nullptr, "1", buffer.data(), buffer.size());
        REQUIRE(result == ELEMENT_OK);
        REQUIRE(strcmp(buffer.data(), "Num") == 0);
        element_interpreter_delete(context);
    }

    SECTION("element_interpreter_typeof_expression twice")
    {
        element_interpreter_ctx* context;
        element_interpreter_create(&context);

        std::string buffer(256, '\0');

        auto result = element_interpreter_typeof_expression(context, nullptr, "1", buffer.data(), buffer.size());
        REQUIRE(result == ELEMENT_OK);
        REQUIRE(strcmp(buffer.data(), "Num") == 0);

        element_interpreter_load_prelude(context);

        buffer.clear();
        buffer.resize(256, '\0');

        result = element_interpreter_typeof_expression(context, nullptr, "Bool(1)", buffer.data(), buffer.size());
        REQUIRE(result == ELEMENT_OK);
        REQUIRE(strcmp(buffer.data(), "Bool") == 0);
        element_interpreter_delete(context);
    }

    SECTION("Compile Time Select")
    {
        element_interpreter_ctx* context;
        element_interpreter_create(&context);

        std::vector<element_value> outputs = { 0 };

        auto selector = std::make_shared<element_expression_constant>(1);
        std::vector<expression_const_shared_ptr> options{
            std::make_shared<const element_expression_constant>(0),
            std::make_shared<const element_expression_constant>(1)
        };

        auto expr = std::make_shared<element_expression_select>(std::move(selector), std::move(options));
        const auto result = element_evaluate(*context, expr, {}, outputs, {});
        REQUIRE(result == ELEMENT_OK);
        REQUIRE(outputs[0] == 1.0f);
    }

    SECTION("Runtime Select")
    {
        element_interpreter_ctx* context;
        element_interpreter_create(&context);

        std::vector<element_value> inputs = { 1 };
        std::vector<element_value> outputs = { 0, 0 };

        auto selector = std::make_shared<element_expression_input>(0);
        std::vector<expression_const_shared_ptr> options{
            std::make_shared<const element_expression_constant>(0),
            std::make_shared<const element_expression_constant>(1)
        };

        auto expr = std::make_shared<element_expression_select>(std::move(selector), std::move(options));
        std::vector<std::pair<std::string, expression_const_shared_ptr>> deps;
        deps.push_back(std::make_pair<std::string, expression_const_shared_ptr>("1", expr));
        deps.push_back(std::make_pair<std::string, expression_const_shared_ptr>("2", expr));
        auto new_expr = std::make_shared<element_expression_structure>(std::move(deps));
        const auto result = element_evaluate(*context, new_expr, inputs, outputs, {});
        REQUIRE(result == ELEMENT_OK);
        REQUIRE(outputs[0] == 1.0f);
        REQUIRE(outputs[1] == 1.0f);
    }
}