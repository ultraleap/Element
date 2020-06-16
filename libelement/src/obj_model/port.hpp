#pragma once

//SELF
#include "type_annotation.hpp"
#include "object.hpp"
#include "ast/types.hpp"

namespace element
{
class port : public object
{
public:
    port() = default;
    explicit port(identifier name, std::unique_ptr<type_annotation> annotation);
    virtual ~port() = default;

    //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
    port(const port& scope) = delete;
    port(port&& scope) = default;
    port& operator=(const port& scope) = delete;
    port& operator=(port&& scope) = delete;

    [[nodiscard]] std::string to_string() const override { return name.value; }
    [[nodiscard]] std::string to_code(int depth) const override;

    identifier name;
    std::unique_ptr<type_annotation> annotation;

private:
    //todo
};
}
