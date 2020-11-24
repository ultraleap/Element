#include "expression_chain.hpp"

//SELF
#include "expression.hpp"
#include "object_model/error.hpp"
#include "object_model/compilation_context.hpp"
#include "instruction_tree/instructions.hpp"

using namespace element;

expression_chain::expression_chain(const declaration* declarer)
    : declarer{ declarer }
{
    expressions.reserve(20);
}

object_const_shared_ptr expression_chain::call(const compilation_context& context,
                                               std::vector<object_const_shared_ptr> compiled_args,
                                               const source_information& source_info) const
{
    //expression_chains are not called, instead, the callstack has the arguments and chains are compiled
    //todo: this should never occur, so keeping the assert is useful. return internal compiler error instead of failed call?
    assert(false);
    return object::call(context, compiled_args, source_info);
}

object_const_shared_ptr expression_chain::compile(const compilation_context& context,
                                                  const source_information& source_info) const
{
    object_const_shared_ptr current = nullptr;
    for (const auto& expression : expressions)
    {
        auto previous = std::move(current); //for debugging
        current = expression->resolve(context, previous.get())->compile(context, source_info);
    }

    current->log_any_error(context.get_logger());
    return current;
}

const scope* expression_chain::get_scope() const
{
    return declarer->our_scope.get();
}