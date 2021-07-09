#pragma once

//STD
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <map>

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

class instruction_nullary_cache
{
public:
    using nullary_map = std::unordered_map<element_nullary_op, std::shared_ptr<const element::instruction_nullary>>;
    
    auto cache(element_nullary_op operation, element::type_const_ptr type)
    {
        return m_cache.emplace(operation, std::make_shared<const element::instruction_nullary>(operation, type));
    };

    std::shared_ptr<const element::instruction_nullary> get(element_nullary_op operation, element::type_const_ptr type)
    {
        if (!m_cache.count(operation))
            cache(operation, type);

        assert(m_cache[operation]->actual_type == type);
        return m_cache[operation];
    }

private:
    nullary_map m_cache;
};

class instruction_constant_cache
{
public:
    using constant_map = std::unordered_map<element_value, std::shared_ptr<const element::instruction_constant>>;
    instruction_constant_cache()
        : m_cached_nan(std::make_shared<const element::instruction_constant>(NAN))
    {
        
    }

    auto cache(element_value value, element::type_const_ptr type)
    {
        if (type == element::type::num.get())
            return m_cache_num.emplace(value, std::make_shared<const element::instruction_constant>(value, type));

        return m_cache_bool.emplace(value, std::make_shared<const element::instruction_constant>(value, type));
    };

    std::shared_ptr<const element::instruction_constant> get(element_value value, element::type_const_ptr type)
    {
        if (std::isnan(value))
            return m_cached_nan;

        if (type == element::type::num.get()) {
            if (!m_cache_num.count(value))
                cache(value, type);
            return m_cache_num[value];
        }

        if (type == element::type::boolean.get()) {
            if (!m_cache_bool.count(value))
                cache(value, type);
            return m_cache_bool[value];
        }

        throw;
    }

private:
    constant_map m_cache_num;
    constant_map m_cache_bool;
    std::shared_ptr<const element::instruction_constant> m_cached_nan;
};

class instruction_serialised_structure_cache
{
public:
    using structure_field_instructions = std::vector<element::instruction_const_shared_ptr>;
    using structure_field_names = std::vector<std::string>;
    struct structure_key
    {
        structure_field_instructions instructions;
        structure_field_names names;
        std::string struct_name;
        bool operator<(const structure_key& other) const noexcept
        {
            if (struct_name == other.struct_name) {
                if (instructions == other.instructions) {
                    return names < other.names;
                }
                return instructions < other.instructions;
            }
            return struct_name < other.struct_name;
        }
    };
    using structure_map = std::map<structure_key, std::shared_ptr<const element::instruction_serialised_structure>>;

    //todo: I really should just set up the instructions properly and use std::set or something
    std::shared_ptr<const element::instruction_serialised_structure> get(
        structure_field_instructions field_instructions,
        structure_field_names field_names,
        std::string struct_name)
    {
        structure_key key{field_instructions, field_names, struct_name};
        auto serialised_structure = std::make_shared<const element::instruction_serialised_structure>(std::move(field_instructions), std::move(field_names), std::move(struct_name));

        auto [cached_value_iterator, inserted] = m_cache.try_emplace(std::move(key), std::move(serialised_structure));
        return cached_value_iterator->second;
    }

private:
    structure_map m_cache;
};

class instruction_select_cache
{
public:
    struct select_key
    {
        element::instruction_const_shared_ptr selector;
        std::vector<element::instruction_const_shared_ptr> options;
        std::string struct_name;
        bool operator<(const select_key& other) const noexcept
        {
            if (selector == other.selector)
                return options < other.options;

            return selector < other.selector;
        }
    };
    using selector_map = std::map<select_key, std::shared_ptr<const element::instruction_select>>;

    //todo: I really should just set up the instructions properly and use std::set or something
    std::shared_ptr<const element::instruction_select> get(
        element::instruction_const_shared_ptr selector,
        std::vector<element::instruction_const_shared_ptr> options)
    {
        select_key key{selector, options};
        auto select = std::make_shared<const element::instruction_select>(std::move(selector), std::move(options));

        auto [cached_value_iterator, inserted] = m_cache.try_emplace(std::move(key), std::move(select));
        return cached_value_iterator->second;
    }

private:
    selector_map m_cache;
};

class instruction_if_cache
{
public:
    struct if_key
    {
        element::instruction_const_shared_ptr predicate;
        element::instruction_const_shared_ptr if_true;
        element::instruction_const_shared_ptr if_false;
        bool operator<(const if_key& other) const noexcept
        {
            if (predicate == other.predicate) {
                if (if_true == other.if_true) {
                    return if_false < other.if_false;
                }
                return if_true < other.if_true;
            }
            return predicate < other.predicate;
        }
    };
    using if_map = std::map<if_key, std::shared_ptr<const element::instruction_if>>;

    //todo: I really should just set up the instructions properly and use std::set or something
    std::shared_ptr<const element::instruction_if> get(
        element::instruction_const_shared_ptr predicate,
        element::instruction_const_shared_ptr if_true,
        element::instruction_const_shared_ptr if_false)
    {
        if_key key{predicate, if_true, if_false};
        auto value = std::make_shared<const element::instruction_if>(std::move(predicate), std::move(if_true), std::move(if_false));

        auto [cached_value_iterator, inserted] = m_cache.try_emplace(std::move(key), std::move(value));
        return cached_value_iterator->second;
    }

private:
    if_map m_cache;
};

class instruction_indexer_cache
{
public:
    struct indexer_key
    {
        std::shared_ptr<const element::instruction_for> for_instruction;
        int index;
        element::type_const_ptr type;
        bool operator<(const indexer_key& other) const noexcept
        {
            if (for_instruction == other.for_instruction) {
                if (index == other.index) {
                    return type < other.type;
                }
                return index < other.index;
            }
            return for_instruction < other.for_instruction;
        }
    };
    using indexer_map = std::map<indexer_key, std::shared_ptr<const element::instruction_indexer>>;

    //todo: I really should just set up the instructions properly and use std::set or something
    std::shared_ptr<const element::instruction_indexer> get(
        std::shared_ptr<const element::instruction_for> for_instruction,
        int index,
        element::type_const_ptr type)
    {
        indexer_key key{for_instruction, index, type};
        auto value = std::make_shared<const element::instruction_indexer>(std::move(for_instruction), index, type);

        auto [cached_value_iterator, inserted] = m_cache.try_emplace(std::move(key), std::move(value));
        return cached_value_iterator->second;
    }

private:
    indexer_map m_cache;
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

    mutable element::scope_caches cache_scope_find;
    mutable instruction_nullary_cache cache_instruction_nullary;
    mutable instruction_constant_cache cache_instruction_constant;
    mutable instruction_serialised_structure_cache cache_instruction_serialised_structure;
    mutable instruction_select_cache cache_instruction_select;
    mutable instruction_if_cache cache_instruction_if;
    mutable instruction_indexer_cache cache_instruction_indexer;
};