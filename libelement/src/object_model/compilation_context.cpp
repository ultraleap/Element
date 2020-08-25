#include "compilation_context.hpp"

using namespace element;

compilation_context::compilation_context(const scope* const scope, element_interpreter_ctx* interpreter)
    : interpreter(interpreter)
    , global_scope{ scope }
{

}

const element_log_ctx* compilation_context::get_logger() const
{
    return interpreter->logger.get();
}