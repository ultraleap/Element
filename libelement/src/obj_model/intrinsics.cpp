#include "intrinsics.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "errors.hpp"
#include "functions.hpp"
#include "types.hpp"
#include "intermediaries.hpp"
#include "etree/expressions.hpp"
#include "etree/evaluator.hpp"

DEFINE_TYPE_ID(element::intrinsic_nullary, 1U << 0);
DEFINE_TYPE_ID(element::intrinsic_unary, 1U << 1);
DEFINE_TYPE_ID(element::intrinsic_binary, 1U << 2);
DEFINE_TYPE_ID(element::intrinsic_if, 1U << 3);
DEFINE_TYPE_ID(element::intrinsic_num_constructor, 1U << 4);
DEFINE_TYPE_ID(element::intrinsic_bool_constructor, 1U << 5);

namespace element
{
    std::shared_ptr<const intrinsic> intrinsic::get_intrinsic(const declaration& declaration)
    {
        const static std::unordered_map<std::string, const std::shared_ptr<const intrinsic>> intrinsic_map
        {
            //types
            { "Num", std::make_shared<intrinsic_num_constructor>() },
            { "Bool", std::make_shared<intrinsic_bool_constructor>() },
            { "List", nullptr },
            { "Tuple", nullptr },

            //functions
            { "Num.add", std::make_shared<intrinsic_binary>(element_binary_op::add) },
            { "Num.sub", std::make_shared<intrinsic_binary>(element_binary_op::sub) },
            { "Num.mul", std::make_shared<intrinsic_binary>(element_binary_op::mul) },
            { "Num.div", std::make_shared<intrinsic_binary>(element_binary_op::div) },

            { "Num.pow", std::make_shared<intrinsic_binary>(element_binary_op::pow) },
            { "Num.rem", std::make_shared<intrinsic_binary>(element_binary_op::rem) },

            { "Num.min", std::make_shared<intrinsic_binary>(element_binary_op::min) },
            { "Num.max", std::make_shared<intrinsic_binary>(element_binary_op::max) },

            { "Num.abs", std::make_shared<intrinsic_unary>(element_unary_op::abs) },
            { "Num.ceil", std::make_shared<intrinsic_unary>(element_unary_op::ceil) },
            { "Num.floor", std::make_shared<intrinsic_unary>(element_unary_op::floor) },

            { "Num.sin", std::make_shared<intrinsic_unary>(element_unary_op::sin) },
            { "Num.cos", std::make_shared<intrinsic_unary>(element_unary_op::cos) },
            { "Num.tan", std::make_shared<intrinsic_unary>(element_unary_op::tan) },

            { "Num.asin", std::make_shared<intrinsic_unary>(element_unary_op::asin) },
            { "Num.acos", std::make_shared<intrinsic_unary>(element_unary_op::acos) },
            { "Num.atan", std::make_shared<intrinsic_unary>(element_unary_op::atan) },

            { "Num.atan2", std::make_shared<intrinsic_binary>(element_binary_op::atan2) },

            { "Num.ln", std::make_shared<intrinsic_unary>(element_unary_op::ln) },
            { "Num.log", std::make_shared<intrinsic_binary>(element_binary_op::log) },

            { "Num.NaN", std::make_shared<intrinsic_nullary>(element_nullary_op::nan) },
            { "Num.PositiveInfinity", std::make_shared<intrinsic_nullary>(element_nullary_op::positive_infinity) },
            { "Num.NegativeInfinity", std::make_shared<intrinsic_nullary>(element_nullary_op::negative_infinity) },

            { "Num.eq", std::make_shared<intrinsic_binary>(element_binary_op::eq, type::boolean.get(), type::num.get(), type::num.get()) },
            { "Num.neq", std::make_shared<intrinsic_binary>(element_binary_op::neq, type::boolean.get(), type::num.get(), type::num.get()) },
            { "Num.lt", std::make_shared<intrinsic_binary>(element_binary_op::lt, type::boolean.get(), type::num.get(), type::num.get()) },
            { "Num.leq", std::make_shared<intrinsic_binary>(element_binary_op::leq, type::boolean.get(), type::num.get(), type::num.get()) },
            { "Num.gt", std::make_shared<intrinsic_binary>(element_binary_op::gt, type::boolean.get(), type::num.get(), type::num.get()) },
            { "Num.geq", std::make_shared<intrinsic_binary>(element_binary_op::geq, type::boolean.get(), type::num.get(), type::num.get()) },

            { "Bool.if", std::make_shared<intrinsic_if>() },
            { "Bool.not", std::make_shared<intrinsic_unary>(element_unary_op::not_, type::boolean.get(), type::boolean.get()) },
            { "Bool.and", std::make_shared<intrinsic_binary>(element_binary_op::and_, type::boolean.get(), type::boolean.get()) },
            { "Bool.or", std::make_shared<intrinsic_binary>(element_binary_op:: or_ , type::boolean.get(), type::boolean.get()) },

            { "True", std::make_shared<intrinsic_nullary>(element_nullary_op::true_value, type::boolean.get()) },
            { "False", std::make_shared<intrinsic_nullary>(element_nullary_op::false_value, type::boolean.get()) },

            { "list", nullptr },
            { "List.fold", nullptr },

            { "memberwise", nullptr },
            { "for", nullptr },
            //{ "persist", nullptr }, //todo: unlikely to be part of the language

            //constraints
            { "Any", nullptr },
        };

        const auto location = declaration.location();

        const auto it = intrinsic_map.find(location);
        if (it != intrinsic_map.end())
            return it->second;

        return nullptr;
    }

    intrinsic::intrinsic(const element_type_id id)
        : rtti_type(id)
    {
    }

    intrinsic_function::intrinsic_function(const element_type_id id, type_const_ptr return_type)
        : intrinsic(id), return_type(return_type)
    {
    }

    intrinsic_nullary::intrinsic_nullary(const element_nullary_op operation, type_const_ptr return_type = type::num.get()):
        intrinsic_function(type_id, return_type)
        , operation(operation)
    {
    }

    std::shared_ptr<object> intrinsic_nullary::compile(const compilation_context& context, const source_information& source_info) const
    {
        return std::make_shared<element_expression_nullary>(
            operation, return_type);
    }

    intrinsic_unary::intrinsic_unary(element_unary_op operation, 
                                     type_const_ptr return_type = type::num.get(), 
                                     type_const_ptr argument_type = type::num.get())
        : intrinsic_function(type_id, return_type)
        , operation(operation)
        , argument_type{argument_type}
    {
    }

    std::shared_ptr<object> intrinsic_unary::compile(const compilation_context& context, const source_information& source_info) const
    {
        const auto& frame = context.stack.frames.back();
        const auto& declarer = *frame.function;
        assert(declarer.inputs.size() == 1);
        assert(frame.compiled_arguments.size() == 1);

        const auto intrinsic = get_intrinsic(declarer);
        assert(intrinsic);
        assert(intrinsic.get() == this);

        auto expr = std::dynamic_pointer_cast<element_expression>(frame.compiled_arguments[0]);
        assert(expr);

        return std::make_shared<element_expression_unary>(
            operation,
            std::move(expr),
            return_type);
    }

    intrinsic_binary::intrinsic_binary(const element_binary_op operation, type_const_ptr return_type = type::num.get(),
                                       type_const_ptr first_argument_type = type::num.get(),
                                       type_const_ptr second_argument_type = type::num.get()):
        intrinsic_function(type_id, return_type)
        , operation(operation)
        , first_argument_type{first_argument_type}
        , second_argument_type{second_argument_type}
    {
    }

    std::shared_ptr<object> intrinsic_binary::compile(const compilation_context& context, const source_information& source_info) const
    {
        const auto& frame = context.stack.frames.back();
        const auto& declarer = *frame.function;
        assert(declarer.inputs.size() == 2);
        assert(frame.compiled_arguments.size() == 2);

        const auto intrinsic = get_intrinsic(declarer);
        assert(intrinsic);
        assert(intrinsic.get() == this);
        
        auto expr1 = std::dynamic_pointer_cast<element_expression>(frame.compiled_arguments[0]);
        auto expr2 = std::dynamic_pointer_cast<element_expression>(frame.compiled_arguments[1]);
        assert(expr1);
        assert(expr2);

        return std::make_shared<element_expression_binary>(
            operation,
            expr1,
            expr2,
            return_type);
    }

    intrinsic_if::intrinsic_if()
        : intrinsic_function(type_id, nullptr)
    {
    }

    std::shared_ptr<object> intrinsic_if::compile(const compilation_context& context, const source_information& source_info) const
    {
        const auto& frame = context.stack.frames.back();
        const auto& declarer = *frame.function;
        assert(declarer.inputs.size() == 3);
        assert(frame.compiled_arguments.size() == 3);

        const auto intrinsic = get_intrinsic(declarer);
        assert(intrinsic);
        assert(intrinsic.get() == this);

        auto pred_expr = std::dynamic_pointer_cast<element_expression>(frame.compiled_arguments[0]);
        assert(pred_expr);

        //todo: hack. we can only do if-expressions if the predicate is a constant. the difficulty is in the branches returning a non-expression type

        std::vector<element_value> outputs = { 0 };
        const auto result = element_evaluate(*context.interpreter, pred_expr, {}, outputs, {});
        assert(result == ELEMENT_OK);

        return outputs[0] > 0 ? frame.compiled_arguments[1] : frame.compiled_arguments[2];

        //TODO: Remove zombie code
        auto true_expr = std::dynamic_pointer_cast<element_expression>(frame.compiled_arguments[1]);
        auto false_expr = std::dynamic_pointer_cast<element_expression>(frame.compiled_arguments[2]);
        assert(true_expr);
        assert(false_expr);

        auto ret = std::make_shared<element_expression_if>(
            pred_expr,
            true_expr,
            false_expr);

        return ret;
    }

    intrinsic_num_constructor::intrinsic_num_constructor()
        : intrinsic_function(type_id, type::num.get())
    {
    }

    std::shared_ptr<object> intrinsic_num_constructor::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<object>> compiled_args,
        const source_information& source_info) const
    {
        auto expr = std::dynamic_pointer_cast<element_expression>(compiled_args[0]);
        assert(expr); //todo: I don't think it could be anything but an expression?
        expr->actual_type = type::num.get();
        return expr;
    }

    intrinsic_bool_constructor::intrinsic_bool_constructor()
        : intrinsic_function(type_id, type::boolean.get())
    {
    }

    std::shared_ptr<object> intrinsic_bool_constructor::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<object>> compiled_args,
        const source_information& source_info) const
    {
        auto& true_decl = *context.get_global_scope()->find(identifier("True"), false);
        auto& false_decl = *context.get_global_scope()->find(identifier("False"), false);

        const auto true_expr = get_intrinsic(true_decl)->compile(context, source_info);
        const auto false_expr = get_intrinsic(false_decl)->compile(context, source_info);

        auto expr = std::dynamic_pointer_cast<element_expression>(compiled_args[0]);
        
        assert(expr); //todo: I think this is accurate
        assert(std::dynamic_pointer_cast<element_expression>(true_expr));
        assert(std::dynamic_pointer_cast<element_expression>(false_expr));

        return std::make_shared<element_expression_if>(
            expr,
            std::dynamic_pointer_cast<element_expression>(true_expr),
            std::dynamic_pointer_cast<element_expression>(false_expr));
    }
}
