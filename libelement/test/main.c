
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
    printf("%s", msg->message);
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
    element_interpreter_delete(context);
    return ELEMENT_OK;
}

int main(int argc, char** argv)
{
#ifndef LEGACY_COMPILER
    element_result result;

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

    result = eval("one = two; two = one; evaluate = one;");
    if (result != ELEMENT_ERROR_CIRCULAR_COMPILATION)
        return result;

    result = eval("evaluate = one;");
    if (result != ELEMENT_ERROR_UNKNOWN)
        return result;

    result = eval("struct MyStruct(a:Num, b:Num) {} evaluate = MyStruct(1, 2).a;");
    if (result != ELEMENT_OK)
        return result;

    result = eval("struct MyStruct(a:Num, b:Num) {} evaluate = MyStruct(1, 2).b;");
    if (result != ELEMENT_OK)
        return result;

    result = eval("struct MyStruct(a:Num, b:Num) {add(s:MyStruct) = s.a.add(s.b);} evaluate = MyStruct(1, 2).add;");
    if (result != ELEMENT_OK)
        return result;

    result = eval("struct MyStruct(a:Num, b:Num) {add(s:MyStruct, s2:MyStruct) = MyStruct(s.a.add(s2.a), s.b.add(s2.b));} evaluate = MyStruct(1, 2).add(MyStruct(1, 2)).b;");
    if (result != ELEMENT_OK)
        return result;

    return 0;
#else

    //todo: good location for these + copy on build
    //char* packages[] = {
    //    "../test/packages/test/"
    //};

    //char* files[] = {
    //    "../test/test.ele",
    //    "../test/test - Copy.ele"
    //};

    element_interpreter_ctx* context;
    element_interpreter_create(&context);
    element_interpreter_load_prelude(context);

    const char* evaluate = "deferredAdd(a, b) = Num.add(a, b);";
    auto result = element_interpreter_load_string(context, evaluate, "<input>");
    if (result != ELEMENT_OK)
        return result;

    const element_function* fn;

    result = element_interpreter_get_function(context, "fakeadd", &fn);
    if (result != ELEMENT_OK)
        return result;

    element_compiled_function* cfn;

    result = element_interpreter_compile_function(context, fn, &cfn, NULL);
    if (result != ELEMENT_OK)
        return result;

    element_value inputs[2] = { 1, 2 };
    element_value outputs[1];

    result = element_interpreter_evaluate_function(context, cfn, inputs, 2, outputs, 1, NULL);
    if (result != ELEMENT_OK)
        return result;

    printf("%f", outputs[0]);

    //element_result result = element_interpreter_load_files(context, files, 1);
    //element_interpreter_load_packages(context, packages, 1);

    element_interpreter_delete(context);

    return 0;

    //Old Code
    /*

    const char* input =
        "Potato1(n:num) : num { return = thing.otherthing; } \n"
        "Potato2(n:num) : num { return = thing(x).otherthing; } \n"
        "Potato3(n:num) : num { return = thing(x).otherthing(y); } \n"
        "Potato4(n:num) : num { return = thing(x).otherthing.another(z).what; } \n"
        "Tomato(n:num) = 10.mul(n); \n"
        "Tomato2 = 5; \n"
        "Tomato4(n:num) = Tomato(n).add(Tomato(5)).add(n);";

    printf("Input\n\n%s\n", input);

    element_tokeniser_ctx* tctx;
    element_tokeniser_create(&tctx);

    element_interpreter_ctx* context;
    element_interpreter_create(&context);

    clock_t start = clock();
    const size_t iters = 100;*/

/*
    for (size_t i = 0; i < iters; ++i) {
        // element_interpreter_load_string(context, input, "<input>");
        // element_interpreter_clear(context);

        element_tokeniser_run(tctx, input, "<input>");
        element_tokeniser_clear(tctx);
    }
*/

    /*for (size_t i = 0; i < iters; ++i) {
        for (int j = 1; j < argc; ++j) {
            // printf("%s: ", argv[i]);

            FILE* f = fopen(argv[j], "rb");
            fseek(f, 0, SEEK_END);
            long pos = ftell(f);
            fseek(f, 0, SEEK_SET);

            char* buf = calloc(pos + 1, sizeof(char));
            fread(buf, sizeof(char), pos, f);
            fclose(f);
            buf[pos] = '\0';

            element_interpreter_load_string(context, buf, argv[j]);
            // printf("OK\n");
            free(buf);
        }

        element_interpreter_load_string(context, input, "<input>");
        if (i + 1 < iters)
            element_interpreter_clear(context);
    }


    clock_t end = clock();
    double t = 1000000.0 * ((double)(end - start) / CLOCKS_PER_SEC) / iters;
    printf("time (us): %lf\n", t);

    const element_function* function = NULL;
    element_compiled_function* cfn = NULL;
    element_value inputs[2] = { 3.0, 4.0 };
    element_value outputs[1];
    element_interpreter_get_function(context, "num.add", &function);
    element_interpreter_compile_function(context, function, &cfn, NULL);
    element_interpreter_evaluate_function(context, cfn, inputs, 2, outputs, 1, NULL);
    printf("add(%f,%f) = %f\n", inputs[0], inputs[1], outputs[0]);

    inputs[0] = 3.14159f / 4.0f;
    element_interpreter_get_function(context, "num.sin", &function);
    element_interpreter_compile_function(context, function, &cfn, NULL);
    element_interpreter_evaluate_function(context, cfn, inputs, 1, outputs, 1, NULL);
    printf("sin(%f) = %f\n", inputs[0], outputs[0]);

    inputs[0] = 2.5f;
    element_interpreter_get_function(context, "Tomato", &function);
    element_interpreter_compile_function(context, function, &cfn, NULL);
    element_interpreter_evaluate_function(context, cfn, inputs, 1, outputs, 1, NULL);
    printf("Tomato(%f) = %f\n", inputs[0], outputs[0]);

    inputs[0] = 2.5f;
    element_interpreter_get_function(context, "Tomato4", &function);
    element_interpreter_compile_function(context, function, &cfn, NULL);
    element_interpreter_evaluate_function(context, cfn, inputs, 1, outputs, 1, NULL);
    printf("Tomato4(%f) = %f\n", inputs[0], outputs[0]);

    element_interpreter_delete(context);
    
    getchar();
    return 0;*/
#endif
}
