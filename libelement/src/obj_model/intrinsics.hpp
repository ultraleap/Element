#pragma once 

#include <memory>
#include <unordered_map>

#include "ast/fwd.hpp"
#include "fwd.hpp"
#include "object.hpp"
#include "typeutil.hpp"

namespace element
{
    class intrinsic : public object, public rtti_type<intrinsic>
    {
    public:
        intrinsic(element_type_id id);

    public:
        static std::shared_ptr<const intrinsic> get_intrinsic(const declaration& declaration);
    };

    class intrinsic_function : public intrinsic
    {
    protected:
        //TODO: this might need to be a constraint_const_shared_ptr
        type_const_shared_ptr return_type;

    public:
        intrinsic_function(element_type_id id, type_const_shared_ptr return_type);
    };

    class intrinsic_nullary final : public intrinsic_function
    {
    public:
        DECLARE_TYPE_ID();

    private:
        element_nullary_op operation;

    public:
        explicit intrinsic_nullary(element_nullary_op operation, type_const_shared_ptr return_type);

        [[nodiscard]] std::shared_ptr<compiled_expression> compile(const compilation_context& context) const override;
        [[nodiscard]] element_nullary_op get_operation() const { return operation; }
    };

    class intrinsic_unary final : public intrinsic_function
    {
    public:
        DECLARE_TYPE_ID();

    private:
        element_unary_op operation;
        //TODO: this might need to be a constraint_const_shared_ptr
        type_const_shared_ptr argument_type;

    public:
        explicit intrinsic_unary(element_unary_op operation, type_const_shared_ptr return_type, type_const_shared_ptr argument_type);

        [[nodiscard]] std::shared_ptr<compiled_expression> compile(const compilation_context& context) const override;
        [[nodiscard]] element_unary_op get_operation() const { return operation; }
    };

    class intrinsic_binary final : public intrinsic_function
    {
    public:
        DECLARE_TYPE_ID();

    private:
        element_binary_op operation;
        //TODO: this might need to be a constraint_const_shared_ptr
        type_const_shared_ptr first_argument_type;
        //TODO: this might need to be a constraint_const_shared_ptr
        type_const_shared_ptr second_argument_type;

    public:
        explicit intrinsic_binary(element_binary_op operation, type_const_shared_ptr return_type, type_const_shared_ptr first_argument_type, type_const_shared_ptr second_argument_type);

        [[nodiscard]] std::shared_ptr<compiled_expression> compile(const compilation_context& context) const override;
        [[nodiscard]] element_binary_op get_operation() const { return operation; }
    };

    ////TODO: Needs to be handled via list with dynamic indexing, this will be insufficient for when we have user input
    //class intrinsic_if final : public intrinsic
    //{
    //    DECLARE_TYPE_ID();

    //    intrinsic_if(type_const_shared_ptr return_type, std::string name)
    //        : intrinsic(type_id, std::move(return_type), std::move(name))
    //    {
    //    }
    //};
}
