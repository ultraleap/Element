#include "element/token.h"
#include "element/ast.h"
#include "element/interpreter.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

int main(int argc, char** argv)
{
    const char* input =
        "Potato1(n:num) : num { return = thing.otherthing; } \n"
        "Potato2(n:num) : num { return = thing(x).otherthing; } \n"
        "Potato3(n:num) : num { return = thing(x).otherthing(y); } \n"
        "Potato4(n:num) : num { return = thing(x).otherthing.another(z).what; } \n"
        "Tomato(n:num) = 10; \n"
        "Tomato2 = 5; \n"
        "Tomato4(n:num) = add(Tomato(n), add(Tomato(5), n));";

    printf("Input\n\n%s\n", input);

    element_tokeniser_ctx* tctx;
    element_tokeniser_create(&tctx);

    element_interpreter_ctx* ictx;
    element_interpreter_create(&ictx);

    clock_t start = clock();
    const size_t iters = 1000;

/*
    for (size_t i = 0; i < iters; ++i) {
        // element_interpreter_load_string(ictx, input, "<input>");
        // element_interpreter_clear(ictx);

        element_tokeniser_run(tctx, input, "<input>");
        element_tokeniser_clear(tctx);
    }
*/


    for (size_t i = 0; i < iters; ++i) {
        for (int i = 1; i < argc; ++i) {
            // printf("%s: ", argv[i]);

            FILE* f = fopen(argv[i], "r");
            fseek(f, 0, SEEK_END);
            long pos = ftell(f);
            fseek(f, 0, SEEK_SET);

            char* buf = calloc(pos + 1, sizeof(char));
            fread(buf, sizeof(char), pos, f);
            fclose(f);

            element_interpreter_load_string(ictx, buf, argv[i]);
            // printf("OK\n");
        }

        element_interpreter_load_string(ictx, input, "<input>");
        element_interpreter_clear(ictx);
    }


    clock_t end = clock();
    double t = 1000000.0 * ((double)(end - start) / CLOCKS_PER_SEC) / iters;
    printf("time (us): %lf\n", t);

    const element_function* fn;
    element_compiled_function* cfn;
    element_value inputs[2] = { 3.0, 4.0 };
    element_value outputs[1];
    element_interpreter_get_function(ictx, "add", &fn);
    element_interpreter_compile_function(ictx, fn, &cfn, NULL);
    element_interpreter_evaluate_function(ictx, cfn, inputs, 2, outputs, 1, NULL);
    printf("add(%f,%f) = %f\n", inputs[0], inputs[1], outputs[0]);

    inputs[0] = 3.14159f / 4.0f;
    element_interpreter_get_function(ictx, "sin", &fn);
    element_interpreter_compile_function(ictx, fn, &cfn, NULL);
    element_interpreter_evaluate_function(ictx, cfn, inputs, 1, outputs, 1, NULL);
    printf("sin(%f) = %f\n", inputs[0], outputs[0]);

    inputs[0] = 2.5f;
    element_interpreter_get_function(ictx, "Tomato", &fn);
    element_interpreter_compile_function(ictx, fn, &cfn, NULL);
    element_interpreter_evaluate_function(ictx, cfn, inputs, 1, outputs, 1, NULL);
    printf("Tomato(%f) = %f\n", inputs[0], outputs[0]);

    inputs[0] = 2.5f;
    element_interpreter_get_function(ictx, "Tomato4", &fn);
    element_interpreter_compile_function(ictx, fn, &cfn, NULL);
    element_interpreter_evaluate_function(ictx, cfn, inputs, 1, outputs, 1, NULL);
    printf("Tomato4(%f) = %f\n", inputs[0], outputs[0]);

    element_interpreter_delete(ictx);
    
    getchar();
    return 0;
}