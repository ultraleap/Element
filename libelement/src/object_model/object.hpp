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
    class object
    {
    public:
        virtual ~object() = default;

        object(const object& scope) = delete;
        object(object&& scope) = delete;
        object& operator=(const object& scope) = delete;
        object& operator=(object&& scope) = delete;

        [[nodiscard]] virtual bool is_constant() const;
        [[nodiscard]] virtual std::string typeof_info() const { return "Unknown"; }
        [[nodiscard]] virtual std::string to_code(int depth) const { return "Unknown"; }
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

        [[nodiscard]] virtual const std::vector<port>& get_inputs() const { static std::vector<port> empty; return empty; };
        [[nodiscard]] virtual const scope* get_scope() const { return nullptr; };
        [[nodiscard]] virtual const std::optional<port>& get_output() const { static std::optional<port> empty; return empty; };
        
        [[nodiscard]] virtual std::shared_ptr<const element_expression> to_expression() const { return nullptr; };

        source_information source_info;

    protected:
        object() = default;
    };

    bool valid_call(const compilation_context& context, const declaration* declarer, const std::vector<object_const_shared_ptr>&compiled_args);
    std::shared_ptr<const error> build_error_for_invalid_call(const compilation_context& context, const declaration* declarer, const std::vector<object_const_shared_ptr>&compiled_args);
    object_const_shared_ptr index_type(const declaration* type,
                                       object_const_shared_ptr instance,
                                       const compilation_context& context,
                                       const identifier& name,
                                       const source_information& source_info);

    object_const_shared_ptr compile_placeholder_expression(const compilation_context& context,
        const object& object,
        const std::vector<port>& inputs,
        element_result& result,
        const source_information& source_info,
        const int placeholder_offset = 0);
}
