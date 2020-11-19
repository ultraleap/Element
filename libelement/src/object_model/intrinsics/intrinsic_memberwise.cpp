#include "intrinsic_memberwise.hpp"

//SELF
#include "object_model/declarations/struct_declaration.hpp"
#include "object_model/declarations/function_declaration.hpp"
#include "object_model/intermediaries/function_instance.hpp"
#include "object_model/intermediaries/struct_instance.hpp"
#include "object_model/error.hpp"


#include <memory>

using namespace element;

intrinsic_memberwise::intrinsic_memberwise()
    : intrinsic_function(type_id, nullptr, true)
{
}

object_const_shared_ptr intrinsic_memberwise::compile(const compilation_context& context,
                                              const source_information& source_info) const
{
    const auto& frame = context.calls.frames.back();
    const int count = static_cast<int>(frame.compiled_arguments.size());

    // TODO: Proper error reporting
    // Check the number of arguments
    assert(count == 3);

    object_const_shared_ptr memberwise_op_obj = frame.compiled_arguments[0];
    const auto memberwise_op_func = std::dynamic_pointer_cast<const function_instance>(memberwise_op_obj);
    int memberwise_op_port_count = memberwise_op_func->declarer->inputs.size();

    // TODO: Proper error checking
    // Check that the memberwise operation is binary
    assert(memberwise_op_port_count == 2);

    object_const_shared_ptr arg_one = frame.compiled_arguments[1];
    object_const_shared_ptr arg_two = frame.compiled_arguments[2];

    std::string type_of_arg_one = arg_one->typeof_info();
    std::string type_of_arg_two = arg_two->typeof_info();

    // TODO: Proper error reporting
    // TODO: Proper string equality checking?
    // Check that the inputs are of the same type
    assert(type_of_arg_one == type_of_arg_two);


    // TODO: What if the declaration is within a namespace??
    const auto* argument_declaration = context.get_global_scope()->find(identifier{type_of_arg_one}, false);
    // Find the declaration of the type, as this is the same as the return type.
    assert(argument_declaration);


    auto arg_struct_decl = dynamic_cast<const struct_declaration*>(argument_declaration);
    bool arg_is_struct = arg_struct_decl != NULL;
    // Check that the two args are containers. This includes being a struct.
    // This includes being a list, as well as a struct
    assert(arg_is_struct);


    const struct_instance* arg_one_instance = dynamic_cast<const struct_instance*>(arg_one.get());
    const struct_instance* arg_two_instance = dynamic_cast<const struct_instance*>(arg_two.get());

    int n_elems = arg_one_instance->fields.size();
    assert(arg_two_instance->fields.size() == n_elems);

    std::vector<object_const_shared_ptr> result_fields(n_elems);
    for(int i=0; i<n_elems; ++i)
    {
        std::string arg_name = argument_declaration->inputs[i].get_name();
        auto field_a = arg_one_instance->fields.at(arg_name);
        auto field_b = arg_two_instance->fields.at(arg_name);
        auto input = std::vector<object_const_shared_ptr> {field_a, field_b};
        auto ret = memberwise_op_func->call(context, input, source_info);
        result_fields[i] = ret;
    }

    // We now have the arguments for the constructor for the result object.
    auto result = arg_struct_decl->call(context, result_fields, source_info);
    return result;
}
