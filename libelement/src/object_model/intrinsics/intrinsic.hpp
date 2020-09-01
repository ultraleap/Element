#pragma once

//STD
#include <unordered_map>
#include <log_errors.hpp>

//SELF
#include "../object.hpp"
#include "typeutil.hpp"
#include "interpreter_internal.hpp"

namespace element
{
    class intrinsic : public object, public rtti_type<intrinsic>
    {
    public:
        intrinsic(element_type_id id);

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth = 0) const override;

        template <typename T>
        static bool register_intrinsic(const element_interpreter_ctx* context, const element_ast* ast, const declaration& declaration);

        static const intrinsic* get_intrinsic(const element_interpreter_ctx* context, const declaration& declaration)
        {
            const auto it = context->intrinsic_map.find(&declaration);
            if (it != context->intrinsic_map.end())
                return it->second.get();

            return nullptr;
        }

        [[nodiscard]] virtual type_const_ptr get_type() const { return nullptr; };

    private:
        const static std::unordered_map<std::string, std::function<std::unique_ptr<const intrinsic, element_interpreter_ctx::Deleter>(const declaration*)>> validation_func_map;
    };
    
    class intrinsic_not_implemented final : public intrinsic
    {
    public:
        DECLARE_TYPE_ID();
        intrinsic_not_implemented() : intrinsic(0) {}
    };

    template <typename T>
    bool intrinsic::register_intrinsic(const element_interpreter_ctx* context, const element_ast* ast, const declaration& declaration)
    {
        if (!declaration.is_intrinsic()) 
        {
            const auto error = element::build_log_error(context->src_context.get(),
                ast, log_error_message_code::intrinsic_not_implemented, declaration.name.value);
            context->logger->log(error);

            return false;
        }

        const auto location = declaration.location();
        const auto it = validation_func_map.find(location);
        if (it == validation_func_map.end())
        {
            const auto error = element::build_log_error(context->src_context.get(),
                ast, log_error_message_code::intrinsic_not_implemented, declaration.name.value);
            context->logger->log(error);

            return false;
        }

        const auto validation_func = it->second;
        auto intrinsic = validation_func(&declaration);
        if (!intrinsic)
        {
            const auto error = element::build_log_error(context->src_context.get(),
                ast, log_error_message_code::intrinsic_type_mismatch, declaration.name.value);
            context->logger->log(error);

            return false;
        }

        using value_type = element_interpreter_ctx::intrinsic_map_type::value_type;
        context->intrinsic_map.insert(value_type{ &declaration, std::move(intrinsic) });
        return true;
    }
}