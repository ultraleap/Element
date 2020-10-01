#pragma once

//SELF
#include "object_model/object.hpp"
#include "etree/expressions.hpp"

namespace element
{
    class struct_instance final : public object, public std::enable_shared_from_this<struct_instance>
    {
    public:
        explicit struct_instance(const struct_declaration* declarer);
        explicit struct_instance(const struct_declaration* declarer, const std::vector<object_const_shared_ptr>& expressions);

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_string() const override;

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] object_const_shared_ptr index(const compilation_context& context,
                                                    const identifier& name,
                                                    const source_information& source_info) const override;

        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                      const source_information& source_info) const override;

        [[nodiscard]] std::shared_ptr<const element_expression> to_expression() const final;

        [[nodiscard]] bool is_constant() const override
        {
            auto is_constant = true;
            for (const auto& [name, item] : fields)
                is_constant &= item->is_constant();

            return is_constant;
        }

        template <typename Callable>
        [[nodiscard]] std::shared_ptr<struct_instance> clone_and_fill_with_expressions(const compilation_context& context, Callable&& callable) const;

        const struct_declaration* const declarer;
        std::map<std::string, object_const_shared_ptr> fields;

    private:
        template <typename Callable>
        [[nodiscard]] std::shared_ptr<struct_instance> clone_and_fill_with_expressions_internal(const compilation_context& context, Callable&& callable, int& index) const;
    };

    template <typename Callable>
    [[nodiscard]] std::shared_ptr<struct_instance> struct_instance::clone_and_fill_with_expressions(const compilation_context& context, Callable&& callable) const
    {
        static_assert(std::is_invocable_v<Callable, const std::string&, const std::shared_ptr<const element_expression>&, int>, "Invalid parameters");
        static_assert(std::is_same_v<
                          std::invoke_result_t<Callable, const std::string&, const std::shared_ptr<const element_expression>&, int>,
                          std::shared_ptr<const element_expression>>,
                      "Invalid return");

        int index = 0;
        return clone_and_fill_with_expressions_internal(context, std::forward<Callable&&>(callable), index);
    }

    template <typename Callable>
    [[nodiscard]] std::shared_ptr<struct_instance> struct_instance::clone_and_fill_with_expressions_internal(const compilation_context& context, Callable&& callable, int& index) const
    {
        auto clone = std::make_shared<struct_instance>(declarer);

        for (const auto& [name, field] : fields)
        {
            const auto* field_type = field->get_constraint();
            const auto* field_type_declarer = field_type->declarer;

            const auto field_as_expression = std::dynamic_pointer_cast<const element_expression>(field);

            if (field_as_expression)
            {
                //if there's no declarer then we're probably dealing with a Num or Bool (IIRC)
                assert(!field_type_declarer);
                auto element = std::invoke(callable, name, field_as_expression, index);
                const auto res = clone->fields.try_emplace(name, std::move(element));
                assert(res.second);
                index += 1;
                continue;
            }

            const auto field_as_instance = std::dynamic_pointer_cast<const struct_instance>(field);
            if (field_as_instance)
            {
                auto sub_clone = field_as_instance->clone_and_fill_with_expressions(context, callable);
                clone->fields.try_emplace(name, std::move(sub_clone));
                continue;
            }

            assert(!"wasn't a num, bool, or stuct, so probably a HOF. not serialisable, caller should check");
            clone->fields.try_emplace(name, std::make_shared<const error>("wasn't a num, bool, or struct, so probably a HOF. not serialisable, caller should check", ELEMENT_ERROR_UNKNOWN, source_information{}));
        }

        return clone;
    }
} // namespace element