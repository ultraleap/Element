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
    template<typename T>
    static bool is_type_of(const declaration* decl)
    {
        return dynamic_cast<const T*>(decl) ? true : false;
    }

    template <typename Class, typename... Args>
    std::unique_ptr<const intrinsic, element_interpreter_ctx::Deleter> make_unique(Args&&... args)
    {
        static_assert(std::is_base_of_v<intrinsic, Class>, "must be derived from intrinsic");
        return std::unique_ptr<const intrinsic, element_interpreter_ctx::Deleter>(static_cast<const intrinsic*>(new Class(std::forward<Args>(args)...)));
    }

    template <typename Class>
    std::unique_ptr<const intrinsic, element_interpreter_ctx::Deleter> make_unique()
    {
        static_assert(std::is_base_of_v<intrinsic, Class>, "must be derived from intrinsic");
        return std::unique_ptr<const intrinsic, element_interpreter_ctx::Deleter>(static_cast<const intrinsic*>(new Class()));
    }

    //TODO: This is a horrible, temporary hack. Remove and replace once we start dealing with constraints
    const std::unordered_map<std::string, std::function<std::unique_ptr<const intrinsic, element_interpreter_ctx::Deleter>(const declaration*)>> intrinsic::validation_func_map
    {
        //type
        { "Num", [&](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? make_unique<intrinsic_num_constructor>() : nullptr); } },
        { "Bool", [&](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? make_unique<intrinsic_bool_constructor>() : nullptr); } },
        { "List", [&](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },
        { "Tuple", [&](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },

        //functions
        { "Num.add", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::add, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
        { "Num.sub", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::sub, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
        { "Num.mul", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::mul, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
        { "Num.div", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::div, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

        { "Num.pow", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::pow, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
        { "Num.rem", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::rem, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

        { "Num.min", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::min, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
        { "Num.max", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::max, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    
        { "Num.abs",   [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::abs, type::num.get(), type::num.get()) : nullptr); } },
        { "Num.ceil",  [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::ceil, type::num.get(), type::num.get()) : nullptr); } },
        { "Num.floor", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::floor, type::num.get(), type::num.get()) : nullptr); } },


        { "Num.sin", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::sin, type::num.get(), type::num.get()) : nullptr); } },
        { "Num.cos", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::cos, type::num.get(), type::num.get()) : nullptr); } },
        { "Num.tan", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::tan, type::num.get(), type::num.get()) : nullptr); } },

        { "Num.asin", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::asin, type::num.get(), type::num.get()) : nullptr); } },
        { "Num.acos", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::acos, type::num.get(), type::num.get()) : nullptr); } },
        { "Num.atan", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::atan, type::num.get(), type::num.get()) : nullptr); } },

        { "Num.atan2", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::atan2, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

        { "Num.ln", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::ln, type::num.get(), type::num.get()) : nullptr); } },
        { "Num.log", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::log, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

        { "Num.NaN", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::nan, type::num.get()) : nullptr); } },
        { "Num.PositiveInfinity", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::positive_infinity, type::num.get()) : nullptr); } },
        { "Num.NegativeInfinity", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::negative_infinity, type::num.get()) : nullptr); } },

        { "Num.eq", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::eq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
        { "Num.neq", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::neq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
        { "Num.lt", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::lt, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
        { "Num.leq", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::leq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
        { "Num.gt", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::gt, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
        { "Num.geq", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::geq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
     
        { "Bool.if", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_if>() : nullptr); } },
        { "Bool.not", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::not_, type::boolean.get(), type::boolean.get()) : nullptr); } },
        { "Bool.and", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::and_, type::boolean.get(), type::boolean.get(), type::boolean.get()) : nullptr); } },
        { "Bool.or", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::or_, type::boolean.get(), type::boolean.get(), type::boolean.get()) : nullptr); } },

        { "True", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::true_value, type::boolean.get()) : nullptr); } },
        { "False", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::false_value, type::boolean.get()) : nullptr); } },

        { "list", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },
        { "List.fold", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },

        { "memberwise", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },
        { "for", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },
        { "persist", [&](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },

        ////constraints
        { "Any", [&](const declaration* decl) { return (is_type_of<constraint_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },
    };

    std::shared_ptr<const element_expression> evaluate(const compilation_context& context, std::shared_ptr<const element_expression> expr)
    {
        std::vector<element_value> outputs = { 0 };
        const auto result = element_evaluate(*context.interpreter, expr, {}, outputs, {});
        if (result != ELEMENT_OK)
            return expr;

        auto new_expr = std::make_unique<element_expression_constant>(outputs[0]);
        new_expr->actual_type = expr->actual_type;
        return new_expr;
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

    std::shared_ptr<const object> intrinsic_nullary::compile(const compilation_context& context,
                                                             const source_information& source_info) const
    {
        return std::make_unique<element_expression_nullary>(
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

    std::shared_ptr<const object> intrinsic_unary::compile(const compilation_context& context,
                                                           const source_information& source_info) const
    {
        const auto& frame = context.calls.frames.back();
        const auto& declarer = *frame.function;
        assert(declarer.inputs.size() == 1);
        assert(frame.compiled_arguments.size() == 1);

        const auto intrinsic = get_intrinsic(context.interpreter, declarer);
        assert(intrinsic);
        assert(intrinsic == this);

        auto expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[0]);
        assert(expr);

        auto new_expr = std::make_unique<element_expression_unary>(
            operation,
            std::move(expr),
            return_type);

        return evaluate(context, std::move(new_expr));
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

    std::shared_ptr<const object> intrinsic_binary::compile(const compilation_context& context,
                                                            const source_information& source_info) const
    {
        const auto& frame = context.calls.frames.back();
        const auto& declarer = *frame.function;
        assert(declarer.inputs.size() == 2);
        assert(frame.compiled_arguments.size() == 2);

        const auto intrinsic = get_intrinsic(context.interpreter, declarer);
        assert(intrinsic);
        assert(intrinsic == this);
        
        auto expr1 = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[0]);
        auto expr2 = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[1]);
        assert(expr1);
        assert(expr2);

        auto new_expr = std::make_unique<element_expression_binary>(
            operation,
            expr1,
            expr2,
            return_type);

        return evaluate(context, std::move(new_expr));
    }

    intrinsic_if::intrinsic_if()
        : intrinsic_function(type_id, nullptr)
    {
    }

    std::shared_ptr<const object> intrinsic_if::compile(const compilation_context& context,
                                                        const source_information& source_info) const
    {
        const auto& frame = context.calls.frames.back();
        const auto& declarer = *frame.function;
        assert(declarer.inputs.size() == 3);
        assert(frame.compiled_arguments.size() == 3);

        const auto intrinsic = get_intrinsic(context.interpreter, declarer);
        assert(intrinsic);
        assert(intrinsic == this);

        auto pred_expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[0]);
        assert(pred_expr);

        //todo: hack. we can only do if-expressions if the predicate is a constant. the difficulty is in the branches returning a non-expression type

        std::vector<element_value> outputs = { 0 };
        const auto result = element_evaluate(*context.interpreter, pred_expr, {}, outputs, {});
        assert(result == ELEMENT_OK);

        return outputs[0] > 0 ? frame.compiled_arguments[1] : frame.compiled_arguments[2];

        //TODO: Remove zombie code
        auto true_expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[1]);
        auto false_expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[2]);
        assert(true_expr);
        assert(false_expr);

        auto ret = std::make_unique<element_expression_if>(
            pred_expr,
            true_expr,
            false_expr);

        return ret;
    }

    intrinsic_num_constructor::intrinsic_num_constructor()
        : intrinsic_function(type_id, type::num.get())
    {
    }

    std::shared_ptr<const object> intrinsic_num_constructor::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<const object>> compiled_args,
        const source_information& source_info) const
    {
        auto expr = std::dynamic_pointer_cast<const element_expression>(compiled_args[0]);
        assert(expr); //todo: I don't think it could be anything but an expression?
        expr->actual_type = type::num.get();

        return evaluate(context, std::move(expr));
    }

    intrinsic_bool_constructor::intrinsic_bool_constructor()
        : intrinsic_function(type_id, type::boolean.get())
    {
    }

    std::shared_ptr<const object> intrinsic_bool_constructor::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<const object>> compiled_args,
        const source_information& source_info) const
    {
        auto& true_decl = *context.get_global_scope()->find(identifier("True"), false);
        auto& false_decl = *context.get_global_scope()->find(identifier("False"), false);

        const auto true_expr = get_intrinsic(context.interpreter, true_decl)->compile(context, source_info);
        const auto false_expr = get_intrinsic(context.interpreter, false_decl)->compile(context, source_info);

        auto expr = std::dynamic_pointer_cast<const element_expression>(compiled_args[0]);
        
        assert(expr); //todo: I think this is accurate
        assert(std::dynamic_pointer_cast<const element_expression>(true_expr));
        assert(std::dynamic_pointer_cast<const element_expression>(false_expr));

        auto new_expr = std::make_unique<element_expression_if>(
            expr,
            std::dynamic_pointer_cast<const element_expression>(true_expr),
            std::dynamic_pointer_cast<const element_expression>(false_expr));

        return evaluate(context, std::move(new_expr));
    }
}
