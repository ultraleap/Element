#pragma once

//SELF
#include "../object.hpp"

namespace element
{
    class struct_instance final : public object, public std::enable_shared_from_this<struct_instance>
    {
    public:
        explicit struct_instance(const struct_declaration* declarer);
        explicit struct_instance(const struct_declaration* declarer, const std::vector<object_const_shared_ptr>& expressions);

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth = 0) const override;

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] object_const_shared_ptr index(const compilation_context& context,
                                                    const identifier& name,
                                                    const source_information& source_info) const override;

        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                      const source_information& source_info) const override;

        [[nodiscard]] std::shared_ptr<const element_expression> to_expression() const final;

        const struct_declaration* const declarer;
        std::map<std::string, object_const_shared_ptr> fields;

    private:
    };
}