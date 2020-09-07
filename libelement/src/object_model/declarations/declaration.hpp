#pragma once

//SELF
#include "object_model/object.hpp"
#include "object_model/scope.hpp"
#include "object_model/call_stack.hpp"

namespace element
{
    static const std::string intrinsic_qualifier = "intrinsic";
    static const std::string namespace_qualifier = "namespace";
    static const std::string constraint_qualifier = "constraint";
    static const std::string struct_qualifier = "struct";
    static const std::string function_qualifier; //empty string
    static const std::string return_keyword = "return";
    static const std::string unidentifier = "_";

    class declaration : public object
    {
    public:
        explicit declaration(identifier name, const scope* parent);

        [[nodiscard]] bool has_inputs() const { return !inputs.empty(); };
        [[nodiscard]] bool has_output() const { return output.has_value(); };
        [[nodiscard]] bool has_constraint() const { return false; }; //TODO: JM - nonsense, this needs to be a constraint::something OR constraint::any
        [[nodiscard]] bool has_scope() const;
        [[nodiscard]] bool is_intrinsic() const { return _intrinsic; }
        [[nodiscard]] virtual bool is_variadic() const { return false; };
        [[nodiscard]] const std::vector<port>& get_inputs() const override { return inputs; }
        [[nodiscard]] const scope* get_scope() const override { return our_scope.get(); };
        [[nodiscard]] const std::optional<port>& get_output() const override { return output; };

        [[nodiscard]] virtual bool serializable(const compilation_context& context) const { return false; };
        [[nodiscard]] virtual bool deserializable(const compilation_context& context) const { return false; };
        [[nodiscard]] virtual object_const_shared_ptr generate_placeholder(const compilation_context& context, int& placeholder_index) const { return nullptr; };

        [[nodiscard]] virtual std::string location() const;
        [[nodiscard]] virtual bool recursive_handler(const call_stack& stack, const declaration* other_declaration, std::vector<call_stack::frame>::const_reverse_iterator iterator) const
        {
            return !is_intrinsic();
        }

        std::string qualifier;
        const identifier name;
        std::vector<port> inputs;
        std::unique_ptr<scope> our_scope; //needed to merge object model
        std::optional<port> output;

    protected:
        bool _intrinsic = false; //todo: we need to decide on coding standards, if our types are lowercase like our variables and functions, we need some way to differentiate them
        //the wrapper is used to return declarations within the object model
        std::shared_ptr<const declaration_wrapper> wrapper;
    };
}