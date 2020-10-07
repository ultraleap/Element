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

struct element_declaration
{
    const element::declaration* decl;
};

struct element_object
{
    std::shared_ptr<const element::object> obj;
};

struct element_metainfo
{
    //todo: decide what kinda stuff to associate here, just need typeof for now
    std::string code;        //the code associated with this thing
    std::string typeof;      //internal type representation
    std::string source_type; //the type as it is in source, i.e. something the user could use in an element source file
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
    void log(element_result message_code, const std::string& message, const std::string& filename = std::string()) const;
    void log(const std::string& message) const;

    struct Deleter
    {
        void operator()(element::intrinsic* i) const;
        void operator()(const element::intrinsic* i) const;
    };

    using intrinsic_map_type = std::unordered_map<const element::declaration*, std::unique_ptr<const element::intrinsic, Deleter>>;
    mutable intrinsic_map_type intrinsic_map;

    bool parse_only = false;
    bool prelude_loaded = false;
    std::shared_ptr<element_log_ctx> logger;
    std::shared_ptr<element::source_context> src_context;
    std::unique_ptr<element::scope> global_scope;
};