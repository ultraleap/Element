#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

#include "element/interpreter.h"
#include "ast/ast_internal.hpp"

#include "construct.hpp"
#include "ast/functions.hpp"
#include "ast/scope.hpp"
#include "ast/types.hpp"
#include "etree/fwd.hpp"
#include "etree/expressions.hpp"


#define LIBELEMENT_CONCAT(a, b) a ## b
#define CREATE_CAST_CONVENIENCE_FUNCTIONS(T) \
static inline bool LIBELEMENT_CONCAT(is_, T)(const construct_shared_ptr& ptr) { return std::dynamic_pointer_cast<LIBELEMENT_CONCAT(element_, T)>(ptr) != nullptr; }\
static inline bool LIBELEMENT_CONCAT(is_, T)(const construct_const_shared_ptr& ptr) { return std::dynamic_pointer_cast<const LIBELEMENT_CONCAT(element_, T)>(ptr) != nullptr; }\
static inline bool LIBELEMENT_CONCAT(is_, T)(const element_construct* ptr) { return dynamic_cast<const LIBELEMENT_CONCAT(element_, T)*>(ptr) != nullptr; }\
static inline LIBELEMENT_CONCAT(T, _shared_ptr) LIBELEMENT_CONCAT(as_, T)(const construct_shared_ptr& ptr) { return std::dynamic_pointer_cast<LIBELEMENT_CONCAT(element_, T)>(ptr); }\
static inline LIBELEMENT_CONCAT(T, _const_shared_ptr) LIBELEMENT_CONCAT(as_, T)(const construct_const_shared_ptr& ptr) { return std::dynamic_pointer_cast<const LIBELEMENT_CONCAT(element_, T)>(ptr); }\
static inline LIBELEMENT_CONCAT(element_, T)* LIBELEMENT_CONCAT(as_, T)(element_construct* ptr) { return dynamic_cast<LIBELEMENT_CONCAT(element_, T)*>(ptr); }\
static inline const LIBELEMENT_CONCAT(element_, T)* LIBELEMENT_CONCAT(as_, T)(const element_construct* ptr) { return dynamic_cast<const LIBELEMENT_CONCAT(element_, T)*>(ptr); }

CREATE_CAST_CONVENIENCE_FUNCTIONS(type_constraint)
CREATE_CAST_CONVENIENCE_FUNCTIONS(type)
CREATE_CAST_CONVENIENCE_FUNCTIONS(function)

#undef CREATE_CAST_CONVENIENCE_FUNCTIONS
#undef LIBELEMENT_CONCAT

struct element_interpreter_options
{
    bool debug;
};

struct element_interpreter_ctx
{
    using LogCallback = void (*)(const element_log_message* const);

    element_interpreter_ctx();

    element_interpreter_options options;
    std::vector<std::pair<std::string, ast_unique_ptr>> trees;
    scope_unique_ptr names;
    std::unordered_map<const element_ast*, const element_scope*> ast_names;
    bool prelude_loaded = false;
    LogCallback log_callback = nullptr;

    element_result load(const char* str, const char* filename = "<input>");
    element_result load_file(const std::string& file);
    element_result load_files(const std::vector<std::string>& files);
    element_result load_package(const std::string& package);
    element_result load_packages(const std::vector<std::string>& packages);
    element_result load_prelude();
    element_result clear();
    element_result print_ast(const std::string& name = "<input>");
    void set_log_callback(LogCallback callback);
    void log(int message_code, const std::string& message, const std::string& filename = std::string());
    void log(const element_log_message& message);
};

struct element_compiled_function
{
    const element_function* function;
    expression_shared_ptr expression;
};