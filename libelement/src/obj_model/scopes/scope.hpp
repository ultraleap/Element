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
        [[nodiscard]] std::shared_ptr<declaration> find(const std::string& identifier, bool recurse) const;

        [[nodiscard]] bool is_root() const
    	{
            return parent_scope == nullptr;
    	}

        [[nodiscard]] const scope* get_global() const
        {
            const scope* global = this;
            while (global->parent_scope)
                global = global->parent_scope;
            return global;
        }
    	
        [[nodiscard]] std::string location() const;
        [[nodiscard]] std::string to_string() const override;
        [[nodiscard]] std::string to_code(int depth = -1) const override;

        element_result merge(std::unique_ptr<scope>&& other)
        {
            if (!is_root() || !other->is_root())
                return ELEMENT_ERROR_UNKNOWN;

            for (auto& [identifier, declaration] : other->declarations)
            {
                if (declarations.count(identifier))
                    return ELEMENT_ERROR_MULTIPLE_DEFINITIONS;

                declarations[identifier] = std::move(declaration);
                if (declarations[identifier]->scope)
                    declarations[identifier]->scope->parent_scope = this;
            }

            return ELEMENT_OK;
        }
    };
}
