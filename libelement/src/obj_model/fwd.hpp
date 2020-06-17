#pragma once

#include <memory>

namespace element
{
    class object;
    class scope;
    class type_annotation;
    class port;

    class expression;
    class identifier_expression;
    class indexing_expression;
    class call_expression;

    class compiled_expression;
    class struct_instance;
    class function_instance;

    class declaration;
    class struct_declaration;
    class constraint_declaration;
    class function_declaration;
    class expression_bodied_function_declaration;
    class namespace_declaration;

    class element_constraint;
    using constraint_shared_ptr = std::shared_ptr<element_constraint>;
    using constraint_const_shared_ptr = std::shared_ptr<const element_constraint>;

    class element_type;
    using type_shared_ptr = std::shared_ptr<element_type>;
    using type_const_shared_ptr = std::shared_ptr<const element_type>;

    struct element_function;
    using function_shared_ptr = std::shared_ptr<element_function>;
    using function_const_shared_ptr = std::shared_ptr<const element_function>;
}