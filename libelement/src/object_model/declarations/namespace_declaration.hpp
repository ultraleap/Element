#pragma once

//SELF
#include "declaration.hpp"
#include "object_model/intermediaries/declaration_wrapper.hpp"

namespace element
{
    class namespace_declaration final : public declaration
    {
    public:
        namespace_declaration(identifier name, const scope* parent_scope);

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(const int depth) const override;
        [[nodiscard]] object_const_shared_ptr index(const compilation_context& context, const identifier& name,
                                                    const source_information& source_info) const override;

        //todo: required because typeof does compilation, might need to change that?
        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                      const source_information& source_info) const override { return wrapper; }

        [[nodiscard]] bool is_intrinsic() const override;
    };
}