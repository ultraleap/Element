#pragma once

//STD
#include <memory>

namespace element
{
    class object;
    //Everything in element is const, so all of our objects should also be const.
    //Be careful when using mutable, make sure it has no affect on compilation.
    using object_const_shared_ptr = std::shared_ptr<const object>;

    class error;
    class compilation_context;

    class scope;
    class type_annotation;
    class port;

    class expression_chain;
    class expression;
    class identifier_expression;
    class indexing_expression;
    class call_expression;
    class anonymous_block_expression;

    class struct_instance;
    class function_instance;
    class anonymous_block_instance;
    class declaration_wrapper;

    class declaration;
    class struct_declaration;
    class constraint_declaration;
    class function_declaration;
    class expression_bodied_function_declaration;
    class namespace_declaration;

    class constraint;
    using constraint_const_ptr = const constraint*;
    using constraint_const_unique_ptr = std::unique_ptr<const constraint>;

    class type;
    using type_const_ptr = const type*;
    using type_const_unique_ptr = std::unique_ptr<const type>;
    class user_type;
    class num_type;
    class bool_type;

    struct element_function;
    using function_shared_ptr = std::shared_ptr<element_function>;
    using function_const_shared_ptr = std::shared_ptr<const element_function>;

    class intrinsic;
    class identifier;
    class source_information;
    class call_stack;
    class capture_stack;
}
