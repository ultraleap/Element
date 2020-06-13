#pragma once

#include <map>
#include <memory>

#include "obj_model/element_object.hpp"
#include "obj_model/declarations/declaration.hpp"

namespace element
{
    struct scope : element_object
	{
        const scope* parent_scope = nullptr;
        const declaration* const declarer;
    	
        std::map<std::string, std::shared_ptr<declaration>> declarations;

        explicit scope(const scope* parent_scope, const declaration* const declarer)
            : parent_scope{ parent_scope }, declarer{ declarer }
        {
        }
    	
        void add_declaration(std::shared_ptr<declaration> declaration);
        std::shared_ptr<element::declaration> find(const std::string& identifier, bool recurse) const;

        [[nodiscard]] bool is_root() const
    	{
            return parent_scope == nullptr;
    	}
    	
        [[nodiscard]] std::string location() const;
        [[nodiscard]] std::string to_string() const override;

        void merge(std::unique_ptr<scope>&& other)
        {
            if (!is_root() || !other->is_root())
                throw;

            for (auto& [identifier, declaration] : other->declarations)
            {
                if (declarations.count(identifier))
                    throw;

                declarations[identifier] = std::move(declaration);
                if (declarations[identifier]->scope)
                    declarations[identifier]->scope->parent_scope = this;
            }
        }
    };
}
