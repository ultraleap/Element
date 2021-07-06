#pragma once

//SELF
#include "object_internal.hpp"

namespace element
{
class error final : public object, public std::enable_shared_from_this<error>
{
public:
    explicit error(std::string message, element_result code, source_information src_info)
        : message{ std::move(message) }
        , code(code)
    {
        source_info = std::move(src_info);
    }

    explicit error(std::string message, element_result code, source_information src_info, const element_log_ctx* logger)
        : message{ std::move(message) }
        , code(code)
    {
        source_info = std::move(src_info);
        log_once(logger);
    }

    [[nodiscard]] bool is_error() const override;
    element_result log_any_error(const element_log_ctx* logger) const override;

    [[nodiscard]] std::string get_name() const override;
    [[nodiscard]] bool is_constant() const override;

    [[nodiscard]] object_const_shared_ptr index(const compilation_context& context,
        const identifier& name,
        const source_information& source_info) const override { return shared_from_this(); }

    [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
        std::vector<object_const_shared_ptr> compiled_args,
        const source_information& source_info) const override { return shared_from_this(); }

    [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
        const source_information& source_info) const override { return shared_from_this(); }

    [[nodiscard]] element_result get_result() const;
    [[nodiscard]] const std::string& get_message() const;
    [[nodiscard]] element_log_message get_log_message() const;

    [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override { return true; }

    element_result log_once(const element_log_ctx* logger) const;

private:
    std::string message;
    element_result code = ELEMENT_ERROR_UNKNOWN;
    mutable bool logged = false;
};
} // namespace element