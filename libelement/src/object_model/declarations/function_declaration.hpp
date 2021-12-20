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
    using body_type = std::variant<std::unique_ptr<object>, const object*>;

    enum class kind {
        expression_bodied,
        scope_bodied,
        intrinsic
    };

    function_declaration(identifier name, const scope* parent_scope, kind function_kind);

    [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
    [[nodiscard]] const constraint* get_constraint() const override;

    [[nodiscard]] std::string typeof_info() const override;
    [[nodiscard]] std::string to_code(const int depth) const override;
    [[nodiscard]] std::string to_code(const int depth, bool include_defaults, bool include_body) const;
    [[nodiscard]] std::string to_string() const override;
    [[nodiscard]] bool is_variadic() const override;

    [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
        std::vector<object_const_shared_ptr> compiled_args,
        const source_information& source_info) const override;

    [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
        const source_information& source_info) const override;

    [[nodiscard]] bool valid_at_boundary(const compilation_context& context) const;
    [[nodiscard]] bool is_intrinsic() const override;

    [[nodiscard]] const body_type& get_body() const;
    void set_body(body_type body);

private:
    body_type body;
    void validate_ports(const compilation_context& context) const;

    mutable bool is_variadic_cached = false;
    mutable bool cached_variadic = false;
    mutable bool ports_validated = false;
    mutable bool valid_ports = false;
    std::unique_ptr<constraint> constraint_;
    kind function_kind;
};
} // namespace element