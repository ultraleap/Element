#pragma once

//STD
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

//SELF
#include "element/interpreter.h"
#include "common_internal.hpp"
#include "object_model/scope.hpp"
#include "object_model/scope_caches.hpp"
#include "instruction_tree/instructions.hpp"
#include "instruction_tree/cache.hpp"

struct element_declaration
{
    const element::declaration* decl = nullptr;
};

struct element_object
{
    std::shared_ptr<const element::object> obj;
};

struct element_instruction
{
    std::shared_ptr<const element::instruction> instruction;
    mutable element::instruction_cache cache;
};

struct element_object_model_ctx
{
    std::unique_ptr<element::compilation_context> ctx;
};

struct element_port
{
    const element::port* port = nullptr;
};

struct element_ports
{
    using port_deleter = void (*)(element_port*);
    std::vector<std::unique_ptr<element_port, port_deleter>> ports;
};

//todo: move somewhere else and add a logger so we don't need to pass interpreter in the C API
struct element_evaluator_ctx
{
    struct boundary
    {
        const element_value* inputs;
        const size_t inputs_count;
    };
    std::vector<boundary> boundaries;
    element_evaluator_options options;
};

struct element_interpreter_ctx
{
public:
    element_interpreter_ctx();

    element_result load_into_scope(const char* str, const char* filename, element::scope*);
    element_result load(const char* str, const char* filename = "<input>");
    element_result load_file(const std::string& file);
    element_result load_files(const std::vector<std::string>& files);
    element_result load_package(const std::string& package);
    element_result load_packages(const std::vector<std::string>& packages);
    element_result load_prelude();
    element_result clear();
    void set_log_callback(LogCallback callback, void* user_data);
    void log(element_result message_code, const std::string& message, const std::string& filename) const;
    void log(element_result code, const std::string& message) const;
    void log(const std::string& message) const;

    element_result call_expression_to_objects(
        const element_compiler_options* options,
        const char* call_expression_string,
        std::vector<element::object_const_shared_ptr>& objects);

    element_result expression_to_object(
        const element_compiler_options* options,
        const char* expression_string,
        element_object** object);

    using intrinsic_map_type = std::unordered_map<const element::declaration*, std::unique_ptr<const element::intrinsic>>;
    mutable intrinsic_map_type intrinsic_map;

    bool parse_only = false;
    bool prelude_loaded = false;
    std::shared_ptr<element_log_ctx> logger;
    std::shared_ptr<element::source_context> src_context;
    std::unique_ptr<element::scope> global_scope;

    mutable element::scope_caches caches;
};