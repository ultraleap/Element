#pragma once

//SELF
#include "typeutil.hpp"
#include "object_model/object_internal.hpp"

namespace element
{
class constraint : public object, public rtti_type<constraint>
{
public:
    static const constraint_const_unique_ptr any;
    //todo: what is a function constraint and what uses it? not something a user has access to, so something internal?
    static const constraint_const_unique_ptr function;

    constraint(element_type_id id, const declaration* declarer)
        : rtti_type(id)
        , declarer(declarer)
    {}

    [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
    [[nodiscard]] const constraint* get_constraint() const override { return this; }

    [[nodiscard]] std::string typeof_info() const override;
    [[nodiscard]] std::string to_code(const int depth) const override;
    [[nodiscard]] std::string get_name() const override;

    const declaration* declarer;
};
} // namespace element