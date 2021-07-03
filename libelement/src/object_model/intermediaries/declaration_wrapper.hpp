#pragma once

//SELF
#include "object_model/declarations/declaration.hpp"

namespace element
{
class declaration_wrapper final : public object
{
public:
    declaration_wrapper(const declaration* declarer)
        : declarer(declarer)
    {}

    [[nodiscard]] std::string get_name() const override { return declarer->get_name(); }
    [[nodiscard]] std::string typeof_info() const override { return declarer->typeof_info(); }
    [[nodiscard]] std::string to_code(const int depth) const override { return declarer->to_code(depth); }
    [[nodiscard]] bool has_inputs() const { return declarer->has_inputs(); }
    [[nodiscard]] bool has_constraint() const { return declarer->has_constraint(); }; //TODO: JM - nonsense, this needs to be a constraint::something OR constraint::any
    [[nodiscard]] bool has_scope() const { return declarer->has_scope(); }
    [[nodiscard]] bool is_intrinsic() const { return declarer->is_intrinsic(); }
    [[nodiscard]] const std::vector<port>& get_inputs() const override { return declarer->get_inputs(); }
    [[nodiscard]] const scope* get_scope() const override { return declarer->get_scope(); };
    [[nodiscard]] const port& get_output() const override { return declarer->get_output(); }
    [[nodiscard]] bool serializable(const compilation_context& context) const { return declarer->serializable(context); }
    [[nodiscard]] bool deserializable(const compilation_context& context) const { return declarer->deserializable(context); }
    [[nodiscard]] object_const_shared_ptr generate_placeholder(const compilation_context& context, std::size_t& placeholder_index, std::size_t boundary_scope) const { return declarer->generate_placeholder(context, placeholder_index, boundary_scope); }
    //[[nodiscard]] std::string location() const { return declarer->location(); }

    [[nodiscard]] object_const_shared_ptr index(const compilation_context& context,
        const identifier& name,
        const source_information& source_info) const override
    {
        return declarer->index(context, name, source_info);
    }

    [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
        std::vector<object_const_shared_ptr> compiled_args,
        const source_information& source_info) const override
    {
        return declarer->call(context, std::move(compiled_args), source_info);
    }

    [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
        const source_information& source_info) const override
    {
        return declarer->compile(context, source_info);
    }

private:
    const declaration* declarer = nullptr;
};
} // namespace element