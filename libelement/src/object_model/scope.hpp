#pragma once

//STD
#include <map>
#include <memory>

//SELF
#include "object.hpp"
#include "declarations/declaration.hpp"

namespace element
{
    class scope final : public object
    {
    public:
        scope(const scope* parent_scope, const object* declaration_or_expression);

        [[nodiscard]] const declaration* find(const identifier& name, bool recurse) const;
        [[nodiscard]] bool is_root() const { return parent_scope == nullptr; }
        [[nodiscard]] bool is_empty() const { return declarations.empty(); }
        [[nodiscard]] const scope* get_global() const;
        [[nodiscard]] const scope* get_parent_scope() const { return parent_scope; }
        [[nodiscard]] std::string typeof_info() const override;

        [[nodiscard]] std::string location() const;
        [[nodiscard]] std::string to_code(int depth = -1) const override;

        [[nodiscard]] bool add_declaration(std::unique_ptr<declaration> declaration);
        element_result merge(std::unique_ptr<scope>&& other);

        //todo: private
        //const declaration* const declarer;
        const object* declaration_or_expression;

    private:
        const scope* parent_scope = nullptr;
        std::map<identifier, std::unique_ptr<declaration>> declarations;
    };
}
