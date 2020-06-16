#pragma once

#include <memory>
#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "obj_model/port.hpp"
#include "obj_model/element_object.hpp"
#include "obj_model/expressions.hpp"

namespace element
{
class scope;

static const std::string intrinsic_qualifier = "intrinsic";
static const std::string namespace_qualifier = "namespace";
static const std::string constraint_qualifier = "constraint";
static const std::string struct_qualifier = "struct";
static const std::string function_qualifier; //empty string
static const std::string return_keyword = "return";

class declaration : public element_object
{
public:
    explicit declaration(identifier name);
    explicit declaration(identifier name, const scope* parent);
    virtual ~declaration() = default;

    //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
    declaration(const declaration& scope) = delete;
    declaration(declaration&& scope) = delete;
    declaration& operator = (const declaration& scope) = delete;
    declaration& operator = (declaration&& scope) = delete;

    [[nodiscard]] bool has_inputs() const { return !inputs.empty(); };
    [[nodiscard]] bool has_output() const { return output.has_value(); };
    [[nodiscard]] bool has_constraint() const { return false; }; //TODO: JM - nonsense, this needs to be a constraint::something OR constraint::any
    [[nodiscard]] bool is_intrinsic() const { return intrinsic; };
    [[nodiscard]] bool has_scope() const;

    [[nodiscard]] virtual std::string location() const;

    bool intrinsic = false;
    std::string qualifier;
    const identifier name;
    std::vector<port> inputs;
    std::unique_ptr<scope> our_scope; //needed to merge object model
    std::optional<port> output;
    //std::unique_ptr<element_constraint> constraint;

private:
    //todo
};

class struct_declaration final : public declaration
{
public:
    struct_declaration(element::identifier identifier, const element::scope* parent_scope, bool is_intrinsic);
    virtual ~struct_declaration() = default;

    //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
    struct_declaration(const struct_declaration& scope) = delete;
    struct_declaration(struct_declaration&& scope) = delete;
    struct_declaration& operator=(const struct_declaration& scope) = delete;
    struct_declaration& operator=(struct_declaration&& scope) = delete;
    
    [[nodiscard]] std::string to_string() const override;
    [[nodiscard]] std::string to_code(int depth) const override;

    [[nodiscard]] std::shared_ptr<element_object> index(const compilation_context& context, const element::identifier&) const override;
};

class constraint_declaration final : public declaration
{
public:
    constraint_declaration(element::identifier identifier, bool is_intrinsic);
    virtual ~constraint_declaration() = default;

    //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
    constraint_declaration(const constraint_declaration& scope) = delete;
    constraint_declaration(constraint_declaration&& scope) = delete;
    constraint_declaration& operator=(const constraint_declaration& scope) = delete;
    constraint_declaration& operator=(constraint_declaration&& scope) = delete;

    [[nodiscard]] std::string to_string() const override;
    [[nodiscard]] std::string to_code(int depth) const override;
};

class function_declaration final : public declaration
{
public:
    function_declaration(identifier identifier, const scope* parent_scope, bool is_intrinsic);
    virtual ~function_declaration() = default;

    //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
    function_declaration(const function_declaration& scope) = delete;
    function_declaration(function_declaration&& scope) = delete;
    function_declaration& operator=(const function_declaration& scope) = delete;
    function_declaration& operator=(function_declaration&& scope) = delete;

    [[nodiscard]] std::string to_string() const override;
    [[nodiscard]] std::string to_code(int depth) const override;

    [[nodiscard]] std::shared_ptr<element_object> call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const override;
    [[nodiscard]] std::shared_ptr<compiled_expression> compile(const compilation_context& context) const override;
};

//expression bodied functions are used as the leaf-functions for a chain of scope bodied ones to prevent recursion
//the last thing in a function call chain must be an expression bodied "return"
class expression_bodied_function_declaration final : public declaration
{
public:
    expression_bodied_function_declaration(identifier identifier, const scope* parent_scope);
    virtual ~expression_bodied_function_declaration() = default;

    //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
    expression_bodied_function_declaration(const expression_bodied_function_declaration& scope) = delete;
    expression_bodied_function_declaration(expression_bodied_function_declaration&& scope) = delete;
    expression_bodied_function_declaration& operator=(const expression_bodied_function_declaration& scope) = delete;
    expression_bodied_function_declaration& operator=(expression_bodied_function_declaration&& scope) = delete;

    [[nodiscard]] std::string to_string() const override;
    [[nodiscard]] std::string to_code(int depth) const override;
    [[nodiscard]] std::shared_ptr<element_object> call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const override;
    [[nodiscard]] std::shared_ptr<compiled_expression> compile(const compilation_context& context) const override;

    std::shared_ptr<expression> expression;
private:
    //todo
};

class namespace_declaration final : public declaration
{
public:
    namespace_declaration(identifier identifier, const element::scope* parent_scope);
    virtual ~namespace_declaration() = default;

    //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
    namespace_declaration(const namespace_declaration& scope) = delete;
    namespace_declaration(namespace_declaration&& scope) = delete;
    namespace_declaration& operator=(const namespace_declaration& scope) = delete;
    namespace_declaration& operator=(namespace_declaration&& scope) = delete;

    [[nodiscard]] std::string to_string() const override;
    [[nodiscard]] std::string to_code(int depth) const override;
    [[nodiscard]] std::shared_ptr<element_object> index(const compilation_context& context, const element::identifier&) const override;
};
}
