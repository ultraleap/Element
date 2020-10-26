#include "intrinsic_constructor_list.hpp"

//SELF
#include "object_model/constraints/type.hpp"
#include "object_model/intermediaries/struct_instance.hpp"
#include "object_model/declarations//struct_declaration.hpp"

using namespace element;

intrinsic_constructor_list::intrinsic_constructor_list()
    : intrinsic_function(type_id, type::num.get())
{
}

object_const_shared_ptr intrinsic_constructor_list::call(
    const compilation_context& context,
    std::vector<object_const_shared_ptr> compiled_args,
    const source_information& source_info) const
{
    const auto* list_decl = context.get_global_scope()->find(identifier{ "List" }, false);
    assert(list_decl);
    const auto* list_struct = static_cast<const struct_declaration*>(list_decl);
    assert(list_struct);
    //todo: type checking in the intrinsic or in the constructor for a struct instance?
    return std::make_unique<struct_instance>(list_struct, compiled_args);
}