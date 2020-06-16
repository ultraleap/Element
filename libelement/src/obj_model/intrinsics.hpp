#pragma once

//STD
#include <utility>

//SELF
#include "object.hpp"
#include "typeutil.hpp"
#include "ast/fwd.hpp"

namespace element
{
    class element_intrinsic
    {
    private:
        type_const_shared_ptr type;
        identifier name;

    protected:
        element_intrinsic(type_const_shared_ptr type, identifier name)
            : type(std::move(type))
            , name(std::move(name))
        {
        }

        [[nodiscard]] identifier get_identifier() const { return name; }
        [[nodiscard]] type_const_shared_ptr get_type() const { return type; }
    };

    class element_intrinsic_nullary : element_intrinsic
    {
    private:
        element_nullary_op operation;

    protected:
        element_intrinsic_nullary(element_nullary_op operation, type_const_shared_ptr type, identifier name)
            : element_intrinsic(std::move(type), std::move(name))
            , operation{ operation }
        {
        }

    public:
        element_nullary_op get_operation() const { return operation; }
    };

    class element_intrinsic_unary : element_intrinsic
    {
    private:
        element_unary_op operation;

    protected:
        element_intrinsic_unary(element_unary_op operation, type_const_shared_ptr type, identifier name)
            : element_intrinsic(std::move(type), std::move(name))
            , operation { operation }
        {
        }

    public:
        element_unary_op get_operation() const { return operation; }
    };

    class element_intrinsic_binary : element_intrinsic
    {
    public:
        DECLARE_TYPE_ID();

    private:
        element_binary_op operation;

    protected:
        element_intrinsic_binary(element_binary_op operation, type_const_shared_ptr type, identifier name)
            : element_intrinsic(std::move(type), std::move(name))
            , operation{ operation }
        {
        }

    public:
        element_binary_op get_operation() const { return operation; }
    };

    //TODO: JM -  Needs to be handled via list with dynamic indexing, this will be insufficient for when we have user input
    class element_intrinsic_if : element_intrinsic
    {
        element_intrinsic_if(type_const_shared_ptr type, identifier name)
            : element_intrinsic(std::move(type), std::move(name))
        {
        }
    };
}
