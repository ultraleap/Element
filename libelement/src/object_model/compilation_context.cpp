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
    auto input_port = port(list_indexer.get(), identifier{ "i" }, std::make_unique<type_annotation>(identifier{ "Num" }));
    list_indexer->inputs.push_back(std::move(input_port));
    bool success = intrinsic::register_intrinsic<function_declaration>(interpreter, nullptr, *list_indexer);
    assert(success);
    const auto* body = intrinsic::get_intrinsic(interpreter, *list_indexer);
    assert(body);
    list_indexer->body = body;
    success = compiler_scope->add_declaration(std::move(list_indexer));
    if (!success)
        throw;
}

const element_log_ctx* compilation_context::get_logger() const
{
    return interpreter->logger.get();
}