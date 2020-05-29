#pragma once

#include <unordered_map>

#include "ast/ast_internal.hpp"
#include "interpreter_internal.hpp"

struct compilation
{
    expression_shared_ptr expression;
    constraint_const_shared_ptr constraint;

    [[nodiscard]] bool valid() const
    {
        return expression.get();
    }
};

struct compilation_cache
{
    struct frame
    {
    friend struct compilation_cache;
    public:
        //todo: enable these as needed, with correct implementations  
        frame(const frame& other) = delete;
        frame(frame&& other) = delete;
        frame& operator=(const frame& other) = delete;
        frame& operator=(frame&& other) = delete;

        ~frame() { m_parent.pop_frame(); }
    protected:
        frame(compilation_cache& parent, const element_scope* function_scope) : m_parent(parent) { m_parent.push_frame(function_scope); }
        compilation_cache& m_parent;
    };

    //todo: is this meant to be some kind of global cache entry? we should pass the global scope to it in that case. I'm not sure we're using it anyway right now
    compilation_cache() { m_cache.resize(1); } //todo: do we still need this?

    //todo: we should probably not allow reassignment of existing function entries
    void add(const element_scope* scope, compilation compiled) { m_cache.back().cache[scope] = std::move(compiled); }

    frame add_frame(const element_scope* function_scope) { return frame(*this, function_scope); }

    const compilation& search(const element_scope* scope) const
    {
        static compilation empty_compilation{};

        for (auto it = m_cache.rbegin(); it != m_cache.rend(); ++it) {
            auto mit = it->cache.find(scope);
            if (mit != it->cache.end())
                return mit->second;
        }

        return empty_compilation;
    }

    //todo: this only makes sense to have here if all the scopes are functions that we've called, so maybe I haven't thought about this enough and it should be its own class
    bool is_callstack_recursive(const element_scope* function_scope)
    {
        for (auto it = m_cache.rbegin(); it != m_cache.rend(); ++it) {
            if (it->function == function_scope)
                return true;
        }

        return false;
    }

private:
    using Cache = std::unordered_map<const element_scope*, compilation>;

    struct CacheEntry
    {
        Cache cache;
        const element_scope* function = nullptr;
    };

    std::vector<CacheEntry> m_cache;

    void push_frame(const element_scope* function_scope) { m_cache.emplace_back(CacheEntry{ Cache{}, function_scope }); }
    void pop_frame() { m_cache.pop_back(); }
};


struct element_compiler_ctx
{
    std::shared_ptr<element_log_ctx> logger;
	
    element_compiler_options options;
    compilation_cache comp_cache;

    void set_log_callback(LogCallback callback)
    {
        logger = std::make_shared<element_log_ctx>();
        logger->callback = callback;
    }
	
    void log(element_result code, const std::string& message, const element_ast* nearest_ast)
    {
        if (logger == nullptr)
            return;

        logger->log(*this, code, message, nearest_ast);
    }

    void log(const std::string& message) const
    {
        if (logger == nullptr)
            return;

        logger->log(message, message_stage::ELEMENT_STAGE_COMPILER);
    }
};

element_result element_compile(
    element_interpreter_ctx& context,
    const element_function* function,
    compilation& output_compilation,
    element_compiler_options opts);
