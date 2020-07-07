#pragma once

//STD
#include <map>
#include <memory>

//SELF
#include "object.hpp"
#include "declarations.hpp"

namespace element
{
    class scope final : public object
    {
    public:
        explicit scope(const scope* parent_scope, const declaration* declarer);
        virtual ~scope() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a unique_ptr
        scope(const scope&) = delete;
        scope(scope&&) = delete;
        scope& operator = (const scope&) = delete;
        scope& operator = (scope&&) = delete;

        [[nodiscard]] std::shared_ptr<declaration> find(const identifier& name, bool recurse) const;
        [[nodiscard]] bool is_root() const { return parent_scope == nullptr; }
        [[nodiscard]] bool is_empty() const { return declarations.empty(); }
        [[nodiscard]] const scope* get_global() const;
        [[nodiscard]] const scope* get_parent_scope() const { return parent_scope; }
        [[nodiscard]] std::string typeof_info() const override;

        [[nodiscard]] std::string location() const;
        [[nodiscard]] std::string to_code(int depth = -1) const override;

        void add_declaration(std::shared_ptr<declaration> declaration);
        element_result merge(std::unique_ptr<scope>&& other);

        const declaration* const declarer;

    private:
        const scope* parent_scope = nullptr;
        std::map<identifier, std::shared_ptr<declaration>> declarations;
    };
}
