#include "element/element.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

int main(int argc, char** argv)
{
    element_declaration* const_int_declaration;
    element_declaration* my_struct_declaration;
    element_object* const_int_obj;
    element_object* my_struct_obj;
    element_interpreter_ctx* interpreter;
    element_object_model_ctx* compilation_ctx;

    element_interpreter_create(&interpreter);
    //element_interpreter_set_log_callback(interpreter, log_callback, nullptr);
    element_interpreter_load_prelude(interpreter);

    const char* input_element = "const_int = 5\n"
                                "struct my_struct(a:Num)\n"
                                "{\n"
                                "func(thing:my_struct, val:Num) = thing.a.mul(val)\n"
                                "}\n";

    element_object* args[1];
    element_object* const_int;
    element_object* my_struct_instance;
    element_object* my_struct_instance_a;
    element_instruction* instruction;

    element_result result = element_interpreter_load_string(interpreter, input_element, "<input>");
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_find(interpreter, "const_int", &const_int_declaration);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_declaration_to_object(const_int_declaration, &const_int_obj);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_object_model_ctx_create(interpreter, &compilation_ctx);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_object_simplify(const_int_obj, compilation_ctx, &const_int);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_find(interpreter, "my_struct", &my_struct_declaration);
    if (result != ELEMENT_OK)
        goto cleanup;

    element_declaration_to_object(my_struct_declaration, &my_struct_obj);

    args[0] = const_int;

    result = element_object_call(my_struct_obj, compilation_ctx, *args, 1, &my_struct_instance);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_object_index(my_struct_instance, compilation_ctx, "a", &my_struct_instance_a);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_object_to_instruction(my_struct_instance_a, &instruction);
    if (result != ELEMENT_OK)
        goto cleanup;

    {
        element_inputs input;
        input.values = NULL;
        input.count = 0;
        element_outputs output;
        float outputs[] = { 0 };
        output.values = outputs;
        output.count = 1;
        result = element_interpreter_evaluate_declaration(interpreter, NULL, instruction, &input, &output);

        if (result != ELEMENT_OK)
            goto cleanup;

        if (outputs[0] == 5)
            printf("Success");
    }

cleanup:
    element_object_delete(&my_struct_instance_a);
    element_object_delete(&my_struct_instance);
    element_object_delete(&my_struct_obj);
    element_object_delete(&const_int_obj);
    element_object_delete(&const_int);
    element_object_model_ctx_delete(&compilation_ctx);
    element_declaration_delete(&my_struct_declaration);
    element_declaration_delete(&const_int_declaration);
    element_interpreter_delete(&interpreter);

    return 0;
}