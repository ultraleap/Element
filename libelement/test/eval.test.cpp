#include <catch2/catch.hpp>

#include "element/token.h"
#include "element/ast.h"
#include "element/interpreter.h"
#include "element/common.h"

//STD
#include <array>
#include <cstring>

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
    element_interpreter_ctx* context= NULL;
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

TEST_CASE("Evaluate", "[Evaluate]")
{
    SECTION("Compiletime - Pass")
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

        //element doesn't support partial application of any function, only instance functions (i.e. member functions with implicit "this")
        /*
        SECTION("Partial application") {
            result = eval("add5 = Num.add(5); evaluate = add5(2);");
            REQUIRE(result == ELEMENT_OK);
        }
         */

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
    }

    SECTION("Runtime")
    {
        element_result result = ELEMENT_OK;

        SECTION("Invalid Boundary Function")
        {
            SECTION("Missing Return Annotation")
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

            SECTION("Missing Port Annotation")
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

            SECTION("Port Annotation is Any")
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

            SECTION("Return Annotation is Any")
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

            SECTION("Return Annotation is Namespace")
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

            SECTION("Returns HigherOrder Function")
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

            SECTION("Returns Struct containing HigherOrder Function")
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

            SECTION("Port is HigherOrder Function")
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

            SECTION("Port is Struct containing HigherOrder Function")
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
            SECTION("Num.add")
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
                    float inputs[] = { 20, 20 };
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
                SECTION("if true")
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

                SECTION("if false")
                {
                    float inputs[] = { -1 };
                    element_inputs input;
                    input.values = inputs;
                    input.count = 1;
                    element_outputs output;
                    float outputs[] = { 0 };
                    output.values = outputs;
                    output.count = 1;

                    char source[] = "evaluate(a:Num):Num = Bool.if(Bool(a), a, a.mul(2));";

                    result = eval_with_inputs(source, &input, &output);

                    REQUIRE(result == ELEMENT_OK);
                    REQUIRE(output.values[0] == input.values[0] * 2);
                }
            }

            SECTION("List")
            {
                element_result result = ELEMENT_OK;
                SECTION("list(a).at(0)")
                {
                    float inputs[] = { -1 };
                    element_inputs input;
                    input.values = inputs;
                    input.count = 1;
                    element_outputs output;
                    float outputs[] = { 0 };
                    output.values = outputs;
                    output.count = 1;

                    char source[] = "evaluate(a:Num):Num = Bool.if(Bool(a), a, a.mul(2));";

                    result = eval_with_inputs("evaluate(a:Num):Num = list(a).at(0);", &input, &output);

                    REQUIRE(result == ELEMENT_OK);
                    REQUIRE(output.values[0] == input.values[0]);
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

    SECTION("Compiletime - Fail")
    {
        element_result result = ELEMENT_OK;

        SECTION("Circular Compilation Error")
        {
            result = eval(
                    "c(d) = a(d);\n"
                    "a(b:Num) = c(b.mul(b));\n"
                    "evaluate = a(5);\n");
            REQUIRE(result == ELEMENT_ERROR_CIRCULAR_COMPILATION);
        }

        SECTION("Identifier Not Found")
        {
            result = eval("evaluate = one;");
            REQUIRE(result == ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);
        }

        SECTION("Cannot Be Used As Instance Function")
        {
            //todo: missing a semi colon is an unfriendly error
            result = eval("struct MyStruct(crap) { f(a:Num) = a; } evaluate = MyStruct(1).f;");
            REQUIRE(result == ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION);
        }

        SECTION("Identifier Not Found - Struct")
        {
            result = eval("struct MyStruct(crap) { f(a:Num) = a; } evaluate = MyStruct(1).a;");
            REQUIRE(result == ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);
        }

        SECTION("Identifier Not Found - Namespace")
        {
            //todo: namespace is "declaration" because that's how the CLI implements typeof, which throws an assert at evaluation (rightly so, shouldn't be an error, because trying to compile it should fail at that stage)
            result = eval("namespace MyNamespace {} evaluate = MyNamespace;");
            REQUIRE(result == ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);
        }

        SECTION("Missing Contents For Call")
        {
            result = eval("namespace MyNamespace {} evaluate = MyNamespace();");
            REQUIRE(result == ELEMENT_ERROR_MISSING_CONTENTS_FOR_CALL);
        }

        SECTION("Failed to call namespace")
        {
            //todo: namespace should have a specific error message for calling a namespace, with a specific error code
            result = eval("namespace MyNamespace {} evaluate = MyNamespace(1);");
            REQUIRE(result == ELEMENT_ERROR_UNKNOWN);
        }

        SECTION("Failed to index function")
        {
            //todo: this should return an error for trying to index a function
            result = eval("func(a) = 1; evaluate = func.a;");
            REQUIRE(result == ELEMENT_ERROR_UNKNOWN);
        }

        SECTION("Partial Grammar")
        {
            result = eval("evaluate\n");
            REQUIRE(result == ELEMENT_ERROR_PARTIAL_GRAMMAR);
        }

        SECTION("Missing Function Body")
        {
            result = eval("evaluate;\n");
            REQUIRE(result == ELEMENT_ERROR_MISSING_FUNCTION_BODY);
        }

        SECTION("Invalid Identifier")
        {
            result = eval(";");
            REQUIRE(result == ELEMENT_ERROR_INVALID_IDENTIFIER);
        }

        SECTION("Invalid Expression")
        {
            result = eval("func = 1; evaluate = func(;);");
            REQUIRE(result == ELEMENT_ERROR_INVALID_EXPRESSION);
        }

        SECTION("Invalid Identifier")
        {
            result = eval("func = 1; evaluate = func(a.;);");
            REQUIRE(result == ELEMENT_ERROR_INVALID_IDENTIFIER);
        }

        SECTION("Missing Parenthesis For Call")
        {
            result = eval("func = 1; evaluate = func(a;);");
            REQUIRE(result == ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL);
        }

        SECTION("Invalid Expression")
        {
            result = eval("func = 1; evaluate = func(a,,);");
            REQUIRE(result == ELEMENT_ERROR_INVALID_EXPRESSION);
        }

        SECTION("Invalid Expression")
        {
            result = eval("func = 1; evaluate = func(a,);");
            REQUIRE(result == ELEMENT_ERROR_INVALID_EXPRESSION);
        }

        SECTION("Invalid Expression")
        {
            result = eval("func = 1; evaluate = func(,);");
            REQUIRE(result == ELEMENT_ERROR_INVALID_EXPRESSION);
        }

        SECTION("Invalid Port")
        {
            result = eval("func(,) = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_PORT);
        }

        SECTION("Invalid Typename")
        {
            result = eval("func(a:,) = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_TYPENAME);
        }

        SECTION("Invalid Typename")
        {
            result = eval("func(a::) = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_TYPENAME);
        }

        SECTION("Invalid Typename")
        {
            result = eval("func(a:) = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_TYPENAME);
        }

        SECTION("Invalid Typename")
        {
            result = eval("func(a: = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_TYPENAME);
        }

        SECTION("Missing Closing Parenthesis For Portlist")
        {
            //note: expressions aren't valid in a typename, might change?
            result = eval("func(a:func(a)) = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_MISSING_CLOSING_PARENTHESIS_FOR_PORTLIST);
        }

        SECTION("Invalid Typename")
        {
            result = eval("func(a:123) = 1; evaluate = func;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_TYPENAME);
        }

        SECTION("Invalid Identifier")
        {
            result = eval("123 = 1;");
            REQUIRE(result == ELEMENT_ERROR_INVALID_IDENTIFIER);
        }

        SECTION("Identifier Not Found")
        {
            //empty is okay, but we look for "evaluate", which produces an error
            result = eval("");
            REQUIRE(result == ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);
        }

        SECTION("Argument Count Mismatch")
        {
            //should log two errors, though we'll only be told the error code for one of them in our API
            result = eval("evaluate = Num.add(1).add(Num.add(1, 2, 3));");
            REQUIRE(result == ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH);
        }

        SECTION("Indexing non-nullary function")
        {
            //todo: better error message
            result = eval("evaluate = 5.add.add(1);");
            REQUIRE(result == ELEMENT_ERROR_UNKNOWN);
        }

        SECTION("Identifier Not Found - Indexing Num")
        {
            //todo: better error message
            result = eval("evaluate = Num.woo;");
            REQUIRE(result == ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);
        }
    }

    SECTION("Compiletime - Fail")
    {
        element_result result = ELEMENT_OK;

        char my_vec[] =
                "struct MyVec(x:Num, y:Num)\n"
                "{\n"
                "    add(a:MyVec, b:MyVec) = MyVec(a.x.add(b.x), a.y.add(b.y));\n"
                "    transform_components(a:MyVec, func) = MyVec(func(a.x), func(a.y), func(a.z));\n"
                "}\n";

        SECTION("Missing Parenthesis For Call")
        {
            result = eval_with_source(my_vec, "evaluate = MyStruct(1, 2, 3).add(MyStruct(4, 5, 6);\n");
            REQUIRE(result == ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL);
        }

        SECTION("Missing Semicolon")
        {
            result = eval_with_source(my_vec, "evaluate = MyStruct(1, 2, 3).add(MyStruct4, 5, 6));\n");
            REQUIRE(result == ELEMENT_ERROR_MISSING_SEMICOLON);
        }

        SECTION("Missing Parenthesis For Call")
        {
            //todo: we should say what the opening bracket was/specifically which "MyStruct" it is referring to
            result = eval_with_source(my_vec, "evaluate = MyStruct(1, 2, 3.add(MyStruct(4, 5, 6));\n");
            REQUIRE(result == ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL);
        }

        SECTION("Missing Function Body")
        {
            //todo: we should print that we expect '=' or '{' when defining the function 'evaluate' but found 'MyStruct' instead
            result = eval_with_source(my_vec, "evaluate MyStruct(1, 2, 3).add(MyStruct(4, 5, 6));\n");
            REQUIRE(result == ELEMENT_ERROR_MISSING_FUNCTION_BODY);
        }
    }

    SECTION("element_interpreter_evaluate_expression once")
    {
        element_interpreter_ctx* context;
        element_interpreter_create(&context);

        element_outputs output;
        float outputs_buffer[] = {0};
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
}