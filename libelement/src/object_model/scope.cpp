#include "scope.hpp"

//SELF
#include "object_model/declarations/declaration.hpp"
#include "object_model/expressions/expression_chain.hpp"
#include "object_model/scope_caches.hpp"
#include "object_model/error.hpp"

using namespace element;

scope::scope(const scope* parent_scope, const object* const declaration_or_expression)
    : /*declaration_or_expression(declaration_or_expression)
    ,*/
    parent_scope(parent_scope)
{
}

element_result scope::merge(std::unique_ptr<scope>&& other)
{
    if (!other->is_root())
        return ELEMENT_ERROR_UNKNOWN;

    for (auto& [identifier, declaration] : other->declarations) {
        if (declarations.count(identifier))
            return ELEMENT_ERROR_MULTIPLE_DEFINITIONS;

        declarations[identifier] = std::move(declaration);
        if (declarations[identifier]->our_scope)
            declarations[identifier]->our_scope->parent_scope = this;
    }

    return ELEMENT_OK;
}

bool scope::mark_declaration_compiler_generated(const identifier& name)
{
    const auto found_it = declarations.find(name);
    if (found_it == declarations.end())
        return false;

    auto decl = std::move(found_it->second);
    decl->name.value = "@" + name.value;
    declarations.erase(found_it);
    auto [it, success] = declarations.try_emplace(decl->name, std::move(decl));
    return success;
}

//std::string scope::location() const
//{
//    if (!declaration_or_expression)
//        return "Not available";
//
//    if (const auto* decl = dynamic_cast<const declaration*>(declaration_or_expression))
//    {
//        return decl->location();
//    }
//
//    //todo: location
//    if (const auto* expr = dynamic_cast<const expression*>(declaration_or_expression))
//    {
//        return *expr->source_info.line_in_source;
//    }
//
//    if (const auto* expr_chain = dynamic_cast<const expression_chain*>(declaration_or_expression))
//    {
//        return *expr_chain->source_info.line_in_source;
//    }
//
//    assert(false);
//    throw;
//    //return declaration_or_expression ? declaration_or_expression->location() : "Not available";
//}

bool scope::add_declaration(std::unique_ptr<declaration> declaration, scope_caches& caches)
{
    auto name = declaration->name.value;
    const auto& [it, success] = declarations.try_emplace(std::move(name), std::move(declaration));

    if (success)
        caches.mark_to_clear();

    return success;
}

bool scope::remove_declaration(const identifier& name, scope_caches& caches)
{
    const bool removed = declarations.erase(name.value) != 0;

    if (removed)
        caches.mark_to_clear();

    return removed;
}

const std::map<identifier, std::unique_ptr<declaration>>& scope::get_declarations() const
{
    return declarations;
}

std::string scope::get_name() const
{
    return "";
}

std::vector<std::string> split(const std::string& full_string, char delimiter = '.')
{
    std::vector<std::string> split_strings;

    size_t start = 0;
    auto end = full_string.find(delimiter);
    //find all but last string
    while (end != std::string::npos) {
        const auto identifier = full_string.substr(start, end - start);
        split_strings.push_back(identifier);

        start = end + 1 /*delimiter legth*/;
        end = full_string.find(delimiter, start);
    }

    split_strings.push_back(full_string.substr(start, full_string.length() - start));
    return split_strings;
}

const declaration* scope::find_identifier(const identifier& name, scope_caches& caches, bool recurse) const
{
    const auto name_it = declarations.find(name);

    if (name_it != declarations.end())
        return name_it->second.get();

    if (recurse && parent_scope)
        return parent_scope->find(name, caches, recurse);

    return nullptr;
};

const declaration* scope::find(const identifier& name, scope_caches& caches, const bool recurse = false) const
{
    auto& cache = caches.get(this);
    const auto name_it = cache.find(name.value);
    if (name_it != cache.end())
        return name_it->second;

    auto split_path = split(name.value);

    const auto* scope = this;
    const declaration* found_decl = nullptr;
    for (const auto& ident : split_path) {
        found_decl = scope->find_identifier(ident, caches, recurse);
        if (!found_decl)
            return nullptr;

        scope = found_decl->our_scope.get();
    }

    cache[name.value] = found_decl;
    return found_decl;
}

const scope* scope::get_global() const
{
    const auto* global = this;
    while (global->parent_scope)
        global = global->parent_scope;

    return global;
}