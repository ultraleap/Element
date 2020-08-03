#pragma once

//SELF
#include "typeutil.hpp"
#include "object.hpp"

namespace element
{
    class constraint : public object, public rtti_type<constraint>
    {
    public:
        //static const constraint_const_unique_ptr nullary;
        //static const constraint_const_unique_ptr unary;
        //static const constraint_const_unique_ptr binary;

        static const constraint_const_unique_ptr any;
        //todo: what is a function constraint and what uses? not something a user has access to, so something internal?
        static const constraint_const_unique_ptr function;

        constraint(element_type_id id, const declaration* declarer)
            : rtti_type(id)
            , declarer(declarer)
        {}

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override { return this; }

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        const declaration* declarer;
    };

    class any_constraint : public constraint
    {
    public:
        DECLARE_TYPE_ID();
        any_constraint() : constraint(type_id, nullptr) {}
    };

    class user_function_constraint : public constraint {
    public:
        DECLARE_TYPE_ID();
        user_function_constraint(const declaration* declarer) : constraint(type_id, declarer) {}

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
    };

    //class nullary_constraint : public constraint
    //{
    //public:
    //    DECLARE_TYPE_ID();

    //    nullary_constraint() : constraint(type_id)
    //    {
    //    }
    //};

    //class unary_constraint : public constraint
    //{
    //public:
    //    DECLARE_TYPE_ID();
    //
    //    unary_constraint() : constraint(type_id)
    //    {
    //    }
    //};
    //
    //class binary_constraint : public constraint {
    //public:
    //    DECLARE_TYPE_ID();
    //
    //    binary_constraint() : constraint(type_id)
    //    {
    //    }
    //};

    class type : public constraint
    {
    public:
        DECLARE_TYPE_ID();

        static const type_const_unique_ptr num;      // the absolute unit
        static const type_const_unique_ptr boolean;

        [[nodiscard]] identifier get_identifier() const { return name; }

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth = 0) const override;

    protected:
        type(element_type_id id, identifier name, const declaration* declarer)
            : constraint(type_id, declarer)
            , name(std::move(name))
        {
        }

        identifier name;
    };

    class num_type : public type
    {
    public:
        DECLARE_TYPE_ID();
        num_type() : type(type_id, name, nullptr)
        {
        }

        [[nodiscard]] std::shared_ptr<const object> index(const compilation_context& context, const identifier& name,
                                                          const source_information& source_info) const override;

    private:
        static identifier name;
    };

    class boolean_type : public type
    {
    public:
        DECLARE_TYPE_ID();
        boolean_type() : type(type_id, name, nullptr)
        {
        }

        [[nodiscard]] std::shared_ptr<const object> index(const compilation_context& context, const identifier& name,
                                                          const source_information& source_info) const override;

    private:
        static identifier name;
    };

    class user_type : public type
    {
    public:
        DECLARE_TYPE_ID();
        user_type(identifier name, const declaration* declarer)
            : type(type_id, name, declarer)
            , declarer(declarer)
            , name(std::move(name))
        {}

        //[[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;

    private:
        const declaration* declarer;
        identifier name;
    };
}