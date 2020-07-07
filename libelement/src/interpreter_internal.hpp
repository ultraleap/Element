#pragma once

#include <vector>
#include <string>
#include <memory>

#include "element/interpreter.h"

#include "ast/scope.hpp"
#include "etree/expressions.hpp"
#include "common_internal.hpp"
#include "obj_model/scope.hpp"

#ifdef LEGACY_COMPILER

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

#else

struct element_compilable
{
    std::shared_ptr<element::object> object;
};

struct element_evaluable
{
    std::shared_ptr<element::object> evaluable;
};

struct element_metainfo
{
    //todo: decide what kinda stuff to associate here, just need typeof for now

    std::string code; //the code associated with this thing
    std::string typeof; //internal type representation
    std::string source_type; //the type as it is in source, i.e. something the user could use in an element source file
};

struct file_information
{
    //note: unique_ptr so it's on the heap and the memory address doesn't change
    std::vector<std::unique_ptr<std::string>> source_lines;
    std::unique_ptr<std::string> file_name;
};

struct element_interpreter_ctx
{
    bool prelude_loaded = false;
    std::shared_ptr<element_log_ctx> logger;

    std::unique_ptr<element::scope> global_scope;

    element_interpreter_ctx();

    element_result load(const char* str, const char* filename = "<input>");
    element_result load_file(const std::string& file);
    element_result load_files(const std::vector<std::string>& files);
    element_result load_package(const std::string& package);
    element_result load_packages(const std::vector<std::string>& packages);
    element_result load_prelude();
    element_result clear();
    void set_log_callback(LogCallback callback);
    void log(int message_code, const std::string& message, const std::string& filename = std::string()) const;
    void log(const std::string& message) const;

    //note: the key is a pointer to the first character of filename stored in the value
    std::map<const char*, file_information> file_info;
};

#endif