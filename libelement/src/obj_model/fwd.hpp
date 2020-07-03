#pragma once

#include <memory>

namespace element
{
    class object;
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
    using constraint_shared_ptr = std::shared_ptr<constraint>;
    using constraint_const_shared_ptr = std::shared_ptr<const constraint>;

    class type;
    using type_shared_ptr = std::shared_ptr<type>;
    using type_const_shared_ptr = std::shared_ptr<const type>;

    struct element_function;
    using function_shared_ptr = std::shared_ptr<element_function>;
    using function_const_shared_ptr = std::shared_ptr<const element_function>;
}
