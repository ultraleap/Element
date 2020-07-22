#pragma once

#include <memory>

namespace element
{
    class object;
    struct identifier;
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

    class struct_instance;
    class function_instance;

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

    struct element_function;
    using function_shared_ptr = std::shared_ptr<element_function>;
    using function_const_shared_ptr = std::shared_ptr<const element_function>;

    class intrinsic;
}
