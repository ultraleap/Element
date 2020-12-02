#pragma once

//SELF
#include "expression.hpp"
#include "object_model/object_internal.hpp"
#include "object_model/capture_stack.hpp"

namespace element
{
    class expression_chain final : public object
    {
    public:
        explicit expression_chain(const declaration* declarer);

        [[nodiscard]] std::string get_name() const override;
        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(const int depth = 0) const override;
        [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
                                                   std::vector<object_const_shared_ptr> compiled_args,
                                                   const source_information& source_info) const override;

        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                      const source_information& source_info) const override;
        [[nodiscard]] const scope* get_scope() const override;

        const declaration* declarer;
        mutable capture_stack captures;
        std::vector<std::unique_ptr<expression>> expressions;

    private:
    };
} // namespace element