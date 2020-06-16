#pragma once

//STD
#include <map>
#include <memory>

//SELF
#include "obj_model/object.hpp"
#include "obj_model/declarations.hpp"

namespace element
{
class scope final : public object
{
public:
    explicit scope(const scope* parent_scope, const declaration* const declarer);
    virtual ~scope() = default;

    //todo: default them if we really need them, but it's unlikely given it should be wrapped in a unique_ptr
    scope(const scope& scope) = delete;
    scope(scope&& scope) = delete;
    scope& operator = (const scope& scope) = delete;
    scope& operator = (scope&& scope) = delete;

    [[nodiscard]] std::shared_ptr<declaration> find(const std::string& name, bool recurse) const;
    [[nodiscard]] bool is_root() const { return parent_scope == nullptr; }
    [[nodiscard]] bool is_empty() const { return declarations.empty(); }
    [[nodiscard]] const scope* get_global() const;
    [[nodiscard]] const scope* get_parent_scope() const { return parent_scope; }
    [[nodiscard]] std::string to_string() const override;

    [[nodiscard]] std::string location() const;
    [[nodiscard]] std::string to_code(int depth = -1) const override;

    void add_declaration(std::shared_ptr<declaration> declaration);
    element_result merge(std::unique_ptr<scope>&& other);

    const declaration* const declarer;

private:
    const scope* parent_scope = nullptr;
    std::map<std::string, std::shared_ptr<declaration>> declarations;
};
}
