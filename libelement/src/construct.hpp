#pragma once

#ifdef LEGACY_COMPILER

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cstdlib>

#include "ast/ast_internal.hpp"
#include "ast/fwd.hpp"


struct element_construct;
using construct_shared_ptr = std::shared_ptr<element_construct>;
using construct_const_shared_ptr = std::shared_ptr<const element_construct>;
using construct_vector = std::vector<construct_shared_ptr>;


struct port_info
{
    std::string name;
    constraint_const_shared_ptr type;

    bool operator==(const port_info& other) const
    {
        return name == other.name && type == other.type;
    }
};


struct element_construct : public std::enable_shared_from_this<element_construct>
{
    // inputs and outputs are lazy-loaded on first access
    // as well as efficiency this is also important logic-wise
    // it allows the object to be in a fully-formed state by the time anyone calls these
    const std::vector<port_info>& inputs() const;
    const std::vector<port_info>& outputs() const;
    inline bool is_leaf() const { return m_inputs.empty() && m_outputs.empty(); }

    const port_info* input(std::string name) const
    {
        for (const auto& p : inputs()) {
            if (p.name == name) return &p;
        }
        return nullptr;
    }

    const port_info* output(std::string name) const
    {
        for (const auto& p : outputs()) {
            if (p.name == name) return &p;
        }
        return nullptr;
    }

    // expression_shared_ptr as_expression() const;

    virtual size_t get_size() const;
    // bool serialize(std::vector<expression_shared_ptr>& output) const;
    // construct_shared_ptr deserialize(const std::vector<expression_shared_ptr>& data) const;
    // construct_shared_ptr deserialize(const std::vector<expression_shared_ptr>& data, size_t& index) const;
    // std::vector<construct_shared_ptr> get_as_array() const;

    virtual ~element_construct() = default;
protected:
    element_construct() = default;

    std::vector<port_info> generate_portlist(const element_scope* scope, const element_ast* portlist) const;
    type_const_shared_ptr find_typename(const element_scope* scope, const element_ast* type) const;

    virtual void generate_ports_cache() const { }
    mutable bool m_ports_cached = false;
    mutable std::vector<port_info> m_inputs;
    mutable std::vector<port_info> m_outputs;

    template <typename Derived>
    std::shared_ptr<Derived> shared_from_base()
    {
        return std::static_pointer_cast<Derived>(shared_from_this());
    }
};

#endif
