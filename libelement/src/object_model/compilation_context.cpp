#include "compilation_context.hpp"

//SELF
#include "declarations/function_declaration.hpp"
#include "expressions/expression_chain.hpp"
#include "intrinsics/intrinsic.hpp"
#include "scope.hpp"

using namespace element;

compilation_context::compilation_context(const scope* const scope, element_interpreter_ctx* interpreter)
    : interpreter(interpreter)
    , global_scope{ scope }
    , compiler_scope(std::make_unique<element::scope>(global_scope, nullptr))
{
    auto list_indexer = std::make_unique<function_declaration>(identifier{ "@list_indexer" }, compiler_scope.get(), function_declaration::kind::intrinsic);
    auto input_port = port(list_indexer.get(), identifier{ "i" }, std::make_unique<type_annotation>(identifier{ "Num" }), nullptr);
    list_indexer->inputs.push_back(std::move(input_port));
    bool success = intrinsic::register_intrinsic<function_declaration>(interpreter, nullptr, *list_indexer);
    if (!success)
        throw; //todo
    const auto* body = intrinsic::get_intrinsic(interpreter, *list_indexer);
    if (!body)
        throw; //todo
    list_indexer->body = body;
    success = compiler_scope->add_declaration(std::move(list_indexer));
    if (!success)
        throw; //todo

    const char* list_fold_src = ""
                                "list_fold(myList:List, initial_value:Any, someFunc:Binary):Any\n"
                                "{\n"
                                "   end_of_list(tuple:Any):Bool = tuple.idx.lt(myList.count)\n"
                                "   accumulate(tuple:Any):Any = {idx = tuple.idx.add(1), accumulated_value = someFunc(tuple.accumulated_value, myList.at(tuple.idx))}\n"
                                "   return:Any = for({idx = 0, accumulated_value = initial_value}, end_of_list, accumulate).accumulated_value\n"
                                "}\n";

    interpreter->load_into_scope(list_fold_src, "compiler_generated_list_fold", compiler_scope.get());
    compiler_scope->mark_declaration_compiler_generated(identifier{ "list_fold" });

    boundaries.push_back({});
}

const element_log_ctx* compilation_context::get_logger() const
{
    return interpreter->logger.get();
}