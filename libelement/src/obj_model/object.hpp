#pragma once

//STD
#include <string>
#include <memory>
#include <utility>
#include <vector>

//SELF
#include "fwd.hpp"

namespace element
{
class compilation_context
{
private:
    const scope* global_scope;

public:
    compilation_context(const scope* const scope);
};

struct identifier
{
    identifier() = default;

    explicit identifier(std::string value)
        : value{std::move(value)}
    {
    }

    static identifier unidentifier;

    identifier(identifier const& other) = default;
    identifier& operator=(identifier const& other) = default;

    identifier(identifier&& other) = default;
    identifier& operator=(identifier&& other) = default;

    ~identifier() = default;

    std::string value;
};

class object
{
public:
    virtual ~object() = default;

    [[nodiscard]] virtual std::string to_string() const { return ""; }
    [[nodiscard]] virtual std::string to_code(int depth) const { return ""; }

    //TODO: Add constraints
    //bool matches_constraint(constraint& constraint);

    //todo: some kind of component architecture?
    [[nodiscard]] virtual std::shared_ptr<object> index(const compilation_context& context, const identifier&) const { return nullptr; };
    [[nodiscard]] virtual std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const { return nullptr; };
    [[nodiscard]] virtual std::shared_ptr<compiled_expression> compile(const compilation_context& context) const { return nullptr; };

protected:
    object() = default;
};

//struct error : object
// {
//    static const object_model_id type_id;
//
//    std::string message;

//    explicit error(std::string message)
//        : object(nullptr, type_id)
//        , message{std::move(message)}
//    {
//    }
//};
}
