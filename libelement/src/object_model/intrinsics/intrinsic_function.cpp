#include "intrinsic_function.hpp"

using namespace element;

intrinsic_function::intrinsic_function(const element_type_id id, type_const_ptr return_type, bool variadic)
    : intrinsic(id), return_type(return_type), variadic(variadic)
{
}