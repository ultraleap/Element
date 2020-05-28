#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "ast/ast_internal.hpp"
#include "ast/fwd.hpp"
#include "ast/scope.hpp"
#include "ast/types.hpp"
#include "construct.hpp"
#include "typeutil.hpp"


struct element_function : public element_construct, public rtti_type<element_function>
{
    // type_shared_ptr type() { return m_type; }
    type_const_shared_ptr type() const { return m_type; }

    virtual std::string name() const = 0;

    static function_const_shared_ptr get_builtin(const std::string& name);

protected:
    element_function(element_type_id id, type_const_shared_ptr type)
        : element_construct()
        , rtti_type(id)
        , m_type(std::move(type))
    {
    }

    void generate_ports_cache() const override;

    type_const_shared_ptr m_type;
};


struct element_intrinsic : public element_function
{
    DECLARE_TYPE_ID();

    std::string name() const override { return m_name; }

protected:
    element_intrinsic(element_type_id id, type_const_shared_ptr type, std::string name)
        : element_function(type_id | id, std::move(type))
        , m_name(std::move(name))
    {
    }

    std::string m_name;
};

struct element_intrinsic_nullary : public element_intrinsic
{
    DECLARE_TYPE_ID();

    element_intrinsic_nullary(element_nullary_op op, type_const_shared_ptr type, std::string name)
        : element_intrinsic(type_id, type, std::move(name))
        , m_op(op)
    {
    }

    element_nullary_op operation() const { return m_op; }

private:
    element_nullary_op m_op;
};

struct element_intrinsic_unary : public element_intrinsic
{
    DECLARE_TYPE_ID();

    element_intrinsic_unary(element_unary_op op, type_const_shared_ptr type, std::string name)
        : element_intrinsic(type_id, type, std::move(name))
        , m_op(op)
    {
    }

    element_unary_op operation() const { return m_op; }

private:
    element_unary_op m_op;
};

struct element_intrinsic_binary : public element_intrinsic
{
    DECLARE_TYPE_ID();

    element_intrinsic_binary(element_binary_op op, type_const_shared_ptr type, std::string name)
        : element_intrinsic(type_id, type, std::move(name))
        , m_op(op)
    {
    }

    element_binary_op operation() const { return m_op; }

private:
    element_binary_op m_op;
};

//TODO: Needs to be handled via list with dynamic indexing, this will be insufficient for when we have user input
struct element_intrinsic_if : public element_intrinsic {
    DECLARE_TYPE_ID();

    element_intrinsic_if(type_const_shared_ptr type, std::string name)
        : element_intrinsic(type_id, std::move(type), std::move(name))
    {
    }
};

// TOOD: expr groups et al


struct element_type_ctor : public element_function
{
    DECLARE_TYPE_ID();

    element_type_ctor(type_const_shared_ptr type)
        : element_function(type_id, std::move(type))
    {
    }

    std::string name() const override { return m_type->name(); }
};


struct element_custom_function : public element_function
{
    DECLARE_TYPE_ID();

    element_custom_function(const element_scope* scope)
        : element_function(type_id, generate_type(scope))
        , m_scope(scope)
    {
    }

    const element_scope* scope() const { return m_scope; }
    std::string name() const override { return scope()->name; }

protected:
    const element_scope* m_scope;

private:
    type_shared_ptr generate_type(const element_scope* scope) const;
};
