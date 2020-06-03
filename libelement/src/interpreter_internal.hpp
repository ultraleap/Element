#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

#include "element/interpreter.h"
#include "ast/ast_internal.hpp"

#include "ast/functions.hpp"
#include "ast/scope.hpp"
#include "etree/fwd.hpp"
#include "etree/expressions.hpp"
#include "common_internal.hpp"

struct element_interpreter_ctx
{
    element_interpreter_ctx();

    std::shared_ptr<element_log_ctx> logger;
    std::vector<std::pair<std::string, ast_unique_ptr>> trees;
    scope_unique_ptr names;
    std::unordered_map<const element_ast*, const element_scope*> ast_names;
    bool prelude_loaded = false;

    element_result load(const char* str, const char* filename = "<input>");
    element_result load_file(const std::string& file);
    element_result load_files(const std::vector<std::string>& files);
    element_result load_package(const std::string& package);
    element_result load_packages(const std::vector<std::string>& packages);
    element_result load_prelude();
    element_result clear();
    void set_log_callback(LogCallback callback);
    void log(int message_code, const std::string& message, const std::string& filename = std::string());
    void log(const std::string& message) const;
};

struct element_compiled_function
{
    const element_function* function;
    expression_shared_ptr expression;
    constraint_const_shared_ptr constraint; //todo: can we enforce a stricker type (e.g. element_type instead of constraint)
};