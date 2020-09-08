#include "intrinsic.hpp"
#include "intrinsic_list_fold.hpp"

//SELF
#include "object_model/error.hpp"
#include "object_model/intermediaries/function_instance.hpp"
#include "object_model/intermediaries/struct_instance.hpp"
#include "object_model/declarations/function_declaration.hpp"
#include "object_model/declarations/struct_declaration.hpp"

using namespace element;

intrinsic_list_fold::intrinsic_list_fold()
    : intrinsic_function(type_id, nullptr)
{
}

object_const_shared_ptr compile_time_fold(const compilation_context& context, 
    const std::shared_ptr<const struct_instance>& list,
    const object_const_shared_ptr& initial,
    const std::shared_ptr<const function_instance>& accumulator_function,
    const source_information& source_info)
{
    const auto is_constant = list->is_constant();
    if (!is_constant)
        return nullptr;

    const auto list_count = list->index(context, identifier::list_count_identifier, source_info);
    if (!list_count->is_constant())
        return nullptr;

    const auto list_count_constant = std::dynamic_pointer_cast<const element_expression_constant>(list_count);

    std::vector<object_const_shared_ptr> indexer_arguments;
    indexer_arguments.resize(1);

    auto aggregate = initial;
    for (int i = 0; i < list_count_constant->value(); ++i)
    {
        indexer_arguments[0] = std::make_shared<const element_expression_constant>(i);
        auto at_index = list->index(context, identifier::list_at_identifier, source_info)->call(context, indexer_arguments, source_info);
        if (!at_index->is_constant())
            return nullptr;

        //note: the order must be maintained across compilers to ensure the same results for non-commutative operations
        aggregate = accumulator_function->call(context, {std::move(aggregate), std::move(at_index)}, source_info);
    }

    return aggregate;
}

object_const_shared_ptr runtime_fold(const compilation_context& context, 
    const std::shared_ptr<const struct_instance>& list,
    const object_const_shared_ptr& initial,
    const std::shared_ptr<const function_instance>& accumulator_function,
    const source_information& source_info)
{
    const auto accumulator_is_boundary = accumulator_function->valid_at_boundary(context);
    if (!accumulator_is_boundary)
        return std::make_shared<const error>("accumulator is not a boundary function", ELEMENT_ERROR_UNKNOWN, accumulator_function->source_info);

    const auto placeholder_offset = 0;
    const auto accumulator_compiled = compile_placeholder_expression(context, *accumulator_function, accumulator_function->declarer->get_inputs(), result, source_info, placeholder_offset);
    if(!accumulator_compiled)
        return std::make_shared<const error>("accumulator failed to compile", result, source_info);

    auto accumulator_expression = accumulator_compiled->to_expression();
    if (!accumulator_expression)
        return std::make_shared<const error>("accumulator failed to compile to an expression tree", ELEMENT_ERROR_UNKNOWN, accumulator_function->source_info);

    //make it work

    return nullptr;
}

object_const_shared_ptr intrinsic_list_fold::compile(const compilation_context& context,
    const source_information& source_info) const
{
    const auto& frame = context.calls.frames.back();
    const auto& declarer = *frame.function;
    assert(declarer.inputs.size() == 3);

    assert(frame.compiled_arguments.size() == 3);

    const auto& list = frame.compiled_arguments[0];
    const auto& initial = frame.compiled_arguments[1];
    const auto& accumulator = frame.compiled_arguments[2];

    auto list_struct = std::dynamic_pointer_cast<const struct_instance>(list);
    if (!list_struct)
        return nullptr;

    auto accumulator_instance = std::dynamic_pointer_cast<const function_instance>(accumulator);
    if (!accumulator_instance)
        return nullptr;

    auto compile_time_result = compile_time_fold(context, list_struct, initial, accumulator_instance, source_info);
    if (compile_time_result)
        return compile_time_result;

    return runtime_fold(context, list_struct, initial, accumulator, source_info);
}





/*
//const char* src = "evaluate(a:Num, b:Num, c:Num, start:Num):Num = list(a, b, c).fold(start, Num.add);";

* accumulator = Num.add;
* magic_struct(index:Num, aggregate:Num){}
* mylist = list(a, b, c);
* predicate(magic:magic_struct):Bool = magic.index.lt(mylist.count);
* body(magic:magic_struct) = magic_struct(magic_struct.index.add(1), accumulator(mylist.at(magic.index), magic.aggreggate);
* evaluate(a:Num, b:Num, c:Num, start:Num):Num = for(magic_struct(0, 0), predicate, body);
*/

/*
intrinsic fold(list:List, initial, accumulator:Binary);
intrinsic for(initial, condition:Predicate, body:Unary);
*/

//auto aggregate = initial;
//   int i = 0;
//   while(i < list.count)
//   {
//       cur_element = list.at(i);
//       aggregate = accumulator.call(aggregate, cur_element);
//       ++i; 
//   }