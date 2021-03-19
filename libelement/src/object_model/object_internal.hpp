#pragma once

//STD
#include <string>
#include <memory>
#include <vector>
#include <optional>

//SELF
#include "ast/fwd.hpp"
#include "fwd.hpp"
#include "port.hpp"
#include "source_information.hpp"

namespace element
{
    /* currently libelement has no way of caching compilation, rewinding the C++ callstack, and then resuming.
     * this causes the C++ callstack to become prohibitively large, and a stack overflow will occur.
     * typically this has been seen with element callstacks approaching ~300 function calls.
     * 100 was chosen as a safe default for how many function calls deep we can be in element, but can be adjusted in the future. */
    constexpr auto reasonable_function_call_limit = 100;

    class object
    {
    public:
        virtual ~object() = default;

        object(const object& scope) = delete;
        object(object&& scope) = delete;
        object& operator=(const object& scope) = delete;
        object& operator=(object&& scope) = delete;

        [[nodiscard]] virtual bool is_error() const;
        virtual element_result log_any_error(const element_log_ctx* logger) const;

        [[nodiscard]] virtual bool is_constant() const;
        [[nodiscard]] virtual std::string typeof_info() const { return "<Unknown>"; }
        [[nodiscard]] virtual std::string to_code(const int depth) const { return "<Unknown>"; }
        [[nodiscard]] virtual std::string to_string() const { return fmt::format("{}:{}", to_code(0), typeof_info()); }
        [[nodiscard]] virtual std::string get_name() const = 0;
        [[nodiscard]] virtual bool matches_constraint(const compilation_context& context, const constraint* constraint) const;
        [[nodiscard]] virtual const constraint* get_constraint() const { return nullptr; };

        [[nodiscard]] virtual object_const_shared_ptr index(const compilation_context& context,
                                                            const identifier& name,
                                                            const source_information& source_info) const;

        [[nodiscard]] virtual object_const_shared_ptr call(const compilation_context& context,
                                                           std::vector<object_const_shared_ptr> compiled_args,
                                                           const source_information& source_info) const;

        [[nodiscard]] virtual object_const_shared_ptr compile(const compilation_context& context,
                                                              const source_information& source_info) const;

        [[nodiscard]] virtual const std::vector<port>& get_inputs() const
        {
            static std::vector<port> empty;
            return empty;
        };
        [[nodiscard]] virtual const scope* get_scope() const { return nullptr; };
        [[nodiscard]] virtual const port& get_output() const
        {
            static port empty;
            return empty;
        };

        [[nodiscard]] virtual std::shared_ptr<const instruction> to_instruction() const { return nullptr; };

        source_information source_info;

    protected:
        object() = default;
    };

    bool valid_call(const compilation_context& context, const declaration* declarer, const std::vector<object_const_shared_ptr>& compiled_args);
    std::shared_ptr<const error> build_error_for_invalid_call(const compilation_context& context, const declaration* declarer, const std::vector<object_const_shared_ptr>& compiled_args);
    object_const_shared_ptr index_type(const declaration* type,
                                       object_const_shared_ptr instance,
                                       const compilation_context& context,
                                       const identifier& name,
                                       const source_information& source_info);

    object_const_shared_ptr compile_placeholder_expression(const compilation_context& context,
                                                           const object& object,
                                                           const std::vector<port>& inputs,
                                                           const source_information& source_info,
                                                           int placeholder_offset = 0,
                                                           int boundary_scope = -1);
} // namespace element
