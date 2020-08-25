#pragma once

//STD
#include <variant>

//SELF
#include "declaration.hpp"
#include "object_model/constraints/constraint.hpp"

namespace element
{
    class function_declaration final : public declaration
    {
    public:
        function_declaration(identifier name, const scope* parent_scope, bool is_intrinsic);

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth) const override;
        [[nodiscard]] bool is_variadic() const override;

        [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
                                                   std::vector<object_const_shared_ptr> compiled_args,
                                                   const source_information& source_info) const override;

        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                      const source_information& source_info) const override;

        [[nodiscard]] bool valid_at_boundary(const compilation_context& context) const;

        std::variant<std::unique_ptr<object>, const object*> body;

    private:
        std::unique_ptr<constraint> constraint_;
    };
}