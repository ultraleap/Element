
#include "element/token.h"
#include "element/ast.h"
#include "element/interpreter.h"
#include "element/common.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

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

    printf("\n----------ELE%d %s\n%d| %s\n%d| %s\n\n%s\n----------\n\n",
        msg->message_code,
        msg->filename,
        msg->line,
        msg->line_in_source ? msg->line_in_source : "",
        msg->line,
        buffer_str,
        msg->message);
}

element_result eval(const char* evaluate)
{
    element_interpreter_ctx* context;
    element_interpreter_create(&context);
    element_interpreter_set_log_callback(context, log_callback);
    element_interpreter_load_prelude(context);

    element_result result = element_interpreter_load_string(context, evaluate, "<input>");
    if (result != ELEMENT_OK)
        return result;

    element_compilable* compilable;
    result = element_interpreter_find(context, "evaluate", &compilable);
    if (result != ELEMENT_OK)
        return result;

    struct element_evaluable* evaluable;
    result = element_interpreter_compile(context, NULL, compilable, &evaluable);
    if (result != ELEMENT_OK)
        return result;

    float inputs[] = { 1, 2 };
    element_inputs input;
    input.values = inputs;
    input.count = 2;

    float outputs[1];
    element_outputs output;
    output.values = outputs;
    output.count = 1;

    result = element_interpreter_evaluate(context, NULL, evaluable, &input, &output);
    if (result != ELEMENT_OK)
        return result;

    printf("%s -> %f\n", evaluate, output.values[0]);

    //todo
    element_delete_compilable(context, &compilable);
    element_delete_evaluable(context, &evaluable);
    element_interpreter_delete(context);
    return ELEMENT_OK;
}

element_result eval_with_source(const char* source, const char* evaluate)
{
    element_interpreter_ctx* context = NULL;
    element_compilable* compilable = NULL;
    element_evaluable* evaluable = NULL;

    element_interpreter_create(&context);
    element_interpreter_set_log_callback(context, log_callback);
    element_interpreter_load_prelude(context);

    element_result result = element_interpreter_load_string(context, source, "<source>");
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_load_string(context, evaluate, "<input>");
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_find(context, "evaluate", &compilable);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_compile(context, NULL, compilable, &evaluable);
    if (result != ELEMENT_OK)
        goto cleanup;

    float inputs[] = { 1, 2 };
    element_inputs input;
    input.values = inputs;
    input.count = 2;

    float outputs[1];
    element_outputs output;
    output.values = outputs;
    output.count = 1;

    result = element_interpreter_evaluate(context, NULL, evaluable, &input, &output);
    if (result != ELEMENT_OK)
        goto cleanup;

    printf("%s -> %f\n", evaluate, output.values[0]);

cleanup:
    element_delete_compilable(context, &compilable);
    element_delete_evaluable(context, &evaluable);
    element_interpreter_delete(context);
    return result;
}

element_result test_failing_evals()
{
    printf("#######Expected to fail#######\n");

    element_result result = ELEMENT_OK;

    result = eval(
        "c(d) = a(d);\n"
        "a(b:Num) = c(b.mul(b));\n"
        "evaluate = a(5);\n");
    if (result != ELEMENT_ERROR_CIRCULAR_COMPILATION)
        return result;

    result = eval("evaluate = one;");
    if (result != ELEMENT_ERROR_IDENTIFIER_NOT_FOUND)
        return result;

    //todo: missing an semi colon is an unfriendly error
    result = eval("struct MyStruct(crap) { f(a:Num) = a; } evaluate = MyStruct(1).f;");
    if (result != ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION)
        return result;

    result = eval("struct MyStruct(crap) { f(a:Num) = a; } evaluate = MyStruct(1).a;");
    if (result != ELEMENT_ERROR_IDENTIFIER_NOT_FOUND)
        return result;

    //todo: namespace is "compilable" because that's how the CLI implements typeof, which throws an assert at evaluation (rightly so, shouldn't be an error, because trying to compile it should fail at that stage)
    /*result = eval("namespace MyNamespace {} evaluate = MyNamespace;");
    if (result != ELEMENT_ERROR_IDENTIFIER_NOT_FOUND)
        return result;*/

    result = eval("namespace MyNamespace {} evaluate = MyNamespace();");
    if (result != ELEMENT_ERROR_MISSING_CONTENTS_FOR_CALL)
        return result;

    //todo: namspace should have a specific error message for calling a namespace, with a specific error code
    result = eval("namespace MyNamespace {} evaluate = MyNamespace(1);");
    if (result != ELEMENT_ERROR_UNKNOWN)
        return result;

    //todo: this should return an error for trying to index a function (only nullaries can be "index", as it actually indexes the thing it returs)
    result = eval("func(a) = 1; evaluate = func.a;");
    if (result != ELEMENT_ERROR_IDENTIFIER_NOT_FOUND)
        return result;

    char my_vec[] =
        "struct MyVec(x:Num, y:Num)\n"
        "{\n"
        "    add(a:MyVec, b:MyVec) = MyVec(a.x.add(b.x), a.y.add(b.y));\n"
        "    transform_components(a:MyVec, func) = MyVec(func(a.x), func(a.y), func(a.z));\n"
        "}\n";

    result = eval_with_source(my_vec, "evaluate = MyStruct(1, 2, 3).add(MyStruct(4, 5, 6);\n");
    if (result != ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL)
        return result;

    result = eval_with_source(my_vec, "evaluate = MyStruct(1, 2, 3).add(MyStruct4, 5, 6));\n");
    if (result != ELEMENT_ERROR_MISSING_SEMICOLON)
        return result;

    //todo: we should say what the opening bracket was/specifically which "MyStruct" it is referring to
    result = eval_with_source(my_vec, "evaluate = MyStruct(1, 2, 3.add(MyStruct(4, 5, 6));\n");
    if (result != ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL)
        return result;

    //todo: we should print that we expect '=' or '{' when defining the function 'evaluate' but found 'MyStruct' instead
    result = eval_with_source(my_vec, "evaluate MyStruct(1, 2, 3).add(MyStruct(4, 5, 6));\n");
    if (result != ELEMENT_ERROR_MISSING_FUNCTION_BODY)
        return result;

    result = eval("evaluate\n");
    if (result != ELEMENT_ERROR_PARTIAL_GRAMMAR)
        return result;

    result = eval("evaluate;\n");
    if (result != ELEMENT_ERROR_MISSING_FUNCTION_BODY)
        return result;

    result = eval(";");
    if (result != ELEMENT_ERROR_INVALID_IDENTIFIER)
        return result;

    result = eval("func = 1; evaluate = func(;);");
    if (result != ELEMENT_ERROR_INVALID_EXPRESSION)
        return result;

    result = eval("func = 1; evaluate = func(a.;);");
    if (result != ELEMENT_ERROR_INVALID_IDENTIFIER)
        return result;

    result = eval("func = 1; evaluate = func(a;);");
    if (result != ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL)
        return result;

    result = eval("func = 1; evaluate = func(a,,);");
    if (result != ELEMENT_ERROR_INVALID_EXPRESSION)
        return result;

    result = eval("func = 1; evaluate = func(a,);");
    if (result != ELEMENT_ERROR_INVALID_EXPRESSION)
        return result;

    result = eval("func = 1; evaluate = func(,);");
    if (result != ELEMENT_ERROR_INVALID_EXPRESSION)
        return result;

    result = eval("func(,) = 1; evaluate = func;");
    if (result != ELEMENT_ERROR_INVALID_PORT)
        return result;

    result = eval("func(a:,) = 1; evaluate = func;");
    if (result != ELEMENT_ERROR_INVALID_TYPENAME)
        return result;

    result = eval("func(a::) = 1; evaluate = func;");
    if (result != ELEMENT_ERROR_INVALID_TYPENAME)
        return result;

    result = eval("func(a:) = 1; evaluate = func;");
    if (result != ELEMENT_ERROR_INVALID_TYPENAME)
        return result;

    result = eval("func(a: = 1; evaluate = func;");
    if (result != ELEMENT_ERROR_INVALID_TYPENAME)
        return result;

    //note: expressions aren't valid in a typename, might change?
    result = eval("func(a:func(a)) = 1; evaluate = func;");
    if (result != ELEMENT_ERROR_MISSING_CLOSING_PARENTHESIS_FOR_PORTLIST)
        return result;

    result = eval("func(a:123) = 1; evaluate = func;");
    if (result != ELEMENT_ERROR_INVALID_TYPENAME)
        return result;

    result = eval("123 = 1;");
    if (result != ELEMENT_ERROR_INVALID_IDENTIFIER)
        return result;

    //empty is okay, but we look for "evaluate", which produces an error
    result = eval("");
    if (result != ELEMENT_ERROR_IDENTIFIER_NOT_FOUND)
        return result;

    printf("#######Failed successfully#######\n");
}

int main(int argc, char** argv)
{
    element_result result = ELEMENT_OK;

    printf("#######Expected to pass#######\n");

    //Expression bodied function. Literal.
    result = eval("evaluate = -3;");
    if (result != ELEMENT_OK)
        return result;

    //intrinsic struct. intrinsic function. calling with arguments
    result = eval("evaluate = Num.add(1, 2);");
    if (result != ELEMENT_OK)
        return result;

    //instance function
    result = eval("evaluate = 1.add(2);");
    if (result != ELEMENT_OK)
        return result;

    //intrinsic nullary
    result = eval("evaluate = Num.NaN;");
    if (result != ELEMENT_OK)
        return result;

    //nullary
    result = eval("evaluate = Num.pi;");
    if (result != ELEMENT_OK)
        return result;

    //nullary which looks up another nullary locally
    result = eval("evaluate = Num.tau;");
    if (result != ELEMENT_OK)
        return result;

    //indexing intrinsic function
    result = eval("evaluate = Num.acos(0).degrees;");
    if (result != ELEMENT_OK)
        return result;

    //indexing nested instance function
    result = eval("evaluate = Num.cos(Num.pi.div(4));");
    if (result != ELEMENT_OK)
        return result;

    //chaining functions. indexing degrees on result of asin. instance function degrees that is now nullary. 
    result = eval("evaluate = Num.asin(-1).degrees;");
    if (result != ELEMENT_OK)
        return result;

    //if expression. false nullary.
    result = eval("evaluate = Bool.if(False, 1, 0);");
    if (result != ELEMENT_OK)
        return result;

    //mod
    result = eval("evaluate = Num.mod(5, 2);");
    if (result != ELEMENT_OK)
        return result;

    //bool constructor
    result = eval("evaluate = Bool(2);");
    if (result != ELEMENT_OK)
        return result;

    //num constructor converts back to num
    result = eval("evaluate = Num(Bool(Num(2))).mul(1);");
    if (result != ELEMENT_OK)
        return result;

    //pass higher order function
    result = eval("double(a:Num) = a.mul(2); applyFive(a) = a(5); evaluate = applyFive(double);");
    if (result != ELEMENT_OK)
        return result;

    //element doesn't support partial application of any function, only instance functions (i.e. member functions with implicit "this")
    /*
    result = eval("add5 = Num.add(5); evaluate = add5(2);");
    if (result != ELEMENT_OK)
        return result;
    */

    //todo: fails
    /*
    result = eval("mul(a) { return(b) = a.mul(b); } evaluate = mul(5)(2);");
    if (result != ELEMENT_OK)
        return result;*/

    //struct fields
    result = eval("struct MyStruct(a:Num, b:Num) {} evaluate = MyStruct(1, 2).a;");
    if (result != ELEMENT_OK)
        return result;

    result = eval("struct MyStruct(a:Num, b:Num) {} evaluate = MyStruct(1, 2).b;");
    if (result != ELEMENT_OK)
        return result;

    //struct instance function
    result = eval("struct MyStruct(a:Num, b:Num) {add(s:MyStruct) = s.a.add(s.b);} evaluate = MyStruct(1, 2).add;");
    if (result != ELEMENT_OK)
        return result;

    //struct instance function called with more parameters
    result = eval("struct MyStruct(a:Num, b:Num) {add(s:MyStruct, s2:MyStruct) = MyStruct(s.a.add(s2.a), s.b.add(s2.b));} evaluate = MyStruct(1, 2).add(MyStruct(1, 2)).b;");
    if (result != ELEMENT_OK)
        return result;

    //typename can refer to things inside scopes
    result = eval("namespace Space { struct Thing(a){}} func(a:Space.Thing) = a.a; evaluate = func(Space.Thing(1));");
    if (result != ELEMENT_OK)
        return result;

    printf("#######Passed Successfully#######\n");

    return test_failing_evals();
}
