#include "intrinsics.hpp"

//LIBS
#include <fmt/format.h>

//SELF
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
DEFINE_TYPE_ID(element::intrinsic_user_constructor, 1U << 6);

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
            { "Bool.not", std::make_shared<intrinsic_unary>(element_unary_op::not, type::boolean.get(), type::boolean.get()) },
            { "Bool.and", std::make_shared<intrinsic_binary>(element_binary_op::and, type::boolean.get(), type::boolean.get()) },
            { "Bool.or", std::make_shared<intrinsic_binary>(element_binary_op:: or , type::boolean.get(), type::boolean.get()) },

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
        : intrinsic(id), return_type(std::move(return_type))
    {
    }

    intrinsic_nullary::intrinsic_nullary(const element_nullary_op operation, type_const_ptr return_type = type::num.get()):
        intrinsic_function(type_id, std::move(return_type))
        , operation(operation)
    {
    }

    std::shared_ptr<object> intrinsic_nullary::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const
    {
        return compile(context);
    }

    std::shared_ptr<object> intrinsic_nullary::compile(const compilation_context& context) const
    {
        auto type = type::num.get();

        if (operation == element_nullary_op::true_value || operation == element_nullary_op::false_value)
            type = type::boolean.get();

        return std::make_shared<element_expression_nullary>(
            operation, type);
    }

    intrinsic_unary::intrinsic_unary(element_unary_op operation, 
                                     type_const_ptr return_type = type::num.get(), 
                                     type_const_ptr argument_type = type::num.get())
        : intrinsic_function(type_id, std::move(return_type))
        , operation(operation)
        , argument_type{std::move(argument_type)}
    {
    }

    std::shared_ptr<object> intrinsic_unary::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const
    {
        auto type = type::num.get();

        if (operation == element_unary_op::not)
            type = type::boolean.get();

        auto expr = std::dynamic_pointer_cast<element_expression>(compiled_args[0]);
        assert(expr);

        return std::make_shared<element_expression_unary>(
            operation,
            std::move(expr),
            type);
    }

    intrinsic_binary::intrinsic_binary(const element_binary_op operation, type_const_ptr return_type = type::num.get(),
                                       type_const_ptr first_argument_type = type::num.get(),
                                       type_const_ptr second_argument_type = type::num.get()):
        intrinsic_function(type_id, std::move(return_type))
        , operation(operation)
        , first_argument_type{std::move(first_argument_type)}
        , second_argument_type{std::move(second_argument_type)}
    {
    }

    std::shared_ptr<object> intrinsic_binary::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const
    {
        auto type = type::num.get();

        switch (operation)
        {
            case element_binary_op::and:
            case element_binary_op::or:
            case element_binary_op::eq:
            case element_binary_op::neq:
            case element_binary_op::lt:
            case element_binary_op::leq:
            case element_binary_op::gt:
            case element_binary_op::geq:
            {
                type = type::boolean.get();
            }
        }

        auto expr1 = std::dynamic_pointer_cast<element_expression>(compiled_args[0]);
        auto expr2 = std::dynamic_pointer_cast<element_expression>(compiled_args[1]);
        assert(expr1);
        assert(expr2);

        return std::make_shared<element_expression_binary>(
            operation,
            expr1,
            expr2,
            std::move(type));
    }

    intrinsic_if::intrinsic_if()
        : intrinsic_function(type_id, nullptr)
    {
    }

    std::shared_ptr<object> intrinsic_if::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const
    {
        auto& frame = context.stack.frames.back();

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

    std::shared_ptr<object> intrinsic_num_constructor::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const
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

    std::shared_ptr<object> intrinsic_bool_constructor::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const
    {
        auto& true_decl = *context.get_global_scope()->find(identifier("True"), false);
        auto& false_decl = *context.get_global_scope()->find(identifier("False"), false);

        const auto true_expr = get_intrinsic(true_decl)->call(context, {});
        const auto false_expr = get_intrinsic(false_decl)->call(context, {});

        auto expr = std::dynamic_pointer_cast<element_expression>(compiled_args[0]);
        
        assert(expr); //todo: I think this is accurate
        assert(std::dynamic_pointer_cast<element_expression>(true_expr));
        assert(std::dynamic_pointer_cast<element_expression>(false_expr));

        return std::make_shared<element_expression_if>(
            expr,
            std::dynamic_pointer_cast<element_expression>(true_expr),
            std::dynamic_pointer_cast<element_expression>(false_expr));
    }

    intrinsic_user_constructor::intrinsic_user_constructor(const struct_declaration* declarer)
        : intrinsic_function(type_id, nullptr)
        , declarer(declarer)
    {
    }

    std::shared_ptr<object> intrinsic_user_constructor::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const
    {
        if (compiled_args.size() != declarer->inputs.size())
        {
            std::string input_params;
            for (unsigned i = 0 ; i < declarer->inputs.size(); ++i)
            {
                const auto& input = declarer->inputs[i];
                input_params += fmt::format("({}) {}:{}", i, input.name.value, input.annotation->name.value);
                if (i != declarer->inputs.size() - 1)
                    input_params += ", ";
            }

            std::string given_params;
            for (unsigned i = 0; i < compiled_args.size(); ++i)
            {
                const auto& input = compiled_args[i];
                given_params += fmt::format("({}) {}", i, input->typeof_info());
                if (i != compiled_args.size() - 1)
                    given_params += ", ";
            }

            return std::make_shared<error>(
                fmt::format("error: attempted to construct an instance of '{}' which requires the parameters '{}', but the parameters of type '{}' were used instead.\n",
                    declarer->location(), input_params, given_params),
                ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH,
                nullptr);
        }

        //todo: check the types of each argument

        return std::make_shared<struct_instance>(declarer, compiled_args);
    }
}
