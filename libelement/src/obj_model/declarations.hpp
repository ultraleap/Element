#pragma once

//STD
#include <memory>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

//SELF
#include "port.hpp"
#include "object.hpp"
#include "expressions.hpp"
#include "types.hpp"
#include "fwd.hpp"

namespace element
{
    static const std::string intrinsic_qualifier = "intrinsic";
    static const std::string namespace_qualifier = "namespace";
    static const std::string constraint_qualifier = "constraint";
    static const std::string struct_qualifier = "struct";
    static const std::string function_qualifier; //empty string
    static const std::string return_keyword = "return";
    static const std::string unidentifier = "_";

    class declaration_compilation_wrapper;

    class declaration : public object
    {
    public:
        explicit declaration(identifier name, const scope* parent);
        virtual ~declaration() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        declaration(const declaration& scope) = delete;
        declaration(declaration&& scope) = delete;
        declaration& operator = (const declaration& scope) = delete;
        declaration& operator = (declaration&& scope) = delete;

        [[nodiscard]] bool has_inputs() const { return !inputs.empty(); };
        [[nodiscard]] bool has_output() const { return output.has_value(); };
        [[nodiscard]] bool has_constraint() const { return false; }; //TODO: JM - nonsense, this needs to be a constraint::something OR constraint::any
        [[nodiscard]] bool has_scope() const;
        [[nodiscard]] bool is_intrinsic() const { return _intrinsic; }
        [[nodiscard]] const std::vector<port>& get_inputs() const override { return inputs; }
        [[nodiscard]] const scope* get_scope() const override { return our_scope.get(); };
        [[nodiscard]] const std::optional<port>& get_output() const override { return output; };

        [[nodiscard]] virtual bool serializable(const compilation_context& context) const { return false; };
        [[nodiscard]] virtual bool deserializable(const compilation_context& context) const { return false; };
        [[nodiscard]] virtual object_const_shared_ptr generate_placeholder(const compilation_context& context, int& placeholder_index) const { return nullptr; };

        [[nodiscard]] virtual std::string location() const;

        std::string qualifier;
        const identifier name;
        std::vector<port> inputs;
        std::unique_ptr<scope> our_scope; //needed to merge object model
        std::optional<port> output;
        //std::unique_ptr<element_constraint> constraint;

    protected:
        bool _intrinsic = false; //todo: we need to decide on coding standards, if our types are lowercase like our variables and functions, we need some way to differentiate them
        //the wrapper is used to return declarations within the object model
        std::shared_ptr<declaration_compilation_wrapper> wrapper;
    };

    class declaration_compilation_wrapper final : public object
    {
    public:
        declaration_compilation_wrapper(const declaration* declarer)
            : declarer(declarer)
        {}

        [[nodiscard]] std::string typeof_info() const override { return declarer->typeof_info(); }
        [[nodiscard]] std::string to_code(int depth) const override { return declarer->to_code(depth); }
        [[nodiscard]] bool has_inputs() const { return declarer->has_inputs(); }
        [[nodiscard]] bool has_output() const { return declarer->has_output(); }
        [[nodiscard]] bool has_constraint() const { return declarer->has_constraint(); }; //TODO: JM - nonsense, this needs to be a constraint::something OR constraint::any
        [[nodiscard]] bool has_scope() const { return declarer->has_scope(); }
        [[nodiscard]] bool is_intrinsic() const { return declarer->is_intrinsic(); }
        [[nodiscard]] const std::vector<port>& get_inputs() const override { return declarer->get_inputs(); }
        [[nodiscard]] const scope* get_scope() const override { return declarer->get_scope(); };
        [[nodiscard]] const std::optional<port>& get_output() const override { return declarer->get_output(); }

        [[nodiscard]] bool serializable(const compilation_context& context) const { return declarer->serializable(context); }
        [[nodiscard]] bool deserializable(const compilation_context& context) const { return declarer->deserializable(context); }
        [[nodiscard]] object_const_shared_ptr generate_placeholder (const compilation_context& context, int& placeholder_index) const { return declarer->generate_placeholder(context, placeholder_index); }

        [[nodiscard]] object_const_shared_ptr index(const compilation_context& context, const identifier& name,
                                                          const source_information& source_info) const { return declarer->index(context, name, source_info); }
        [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
                                                         std::vector<object_const_shared_ptr> compiled_args,
                                                         const source_information& source_info) const { return declarer->call(context, std::move(compiled_args), source_info); }
        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                            const source_information& source_info) const { return declarer->compile(context, source_info); }

        [[nodiscard]] std::string location() const { return declarer->location(); }

    private:
        const declaration* declarer = nullptr;
    };

    class struct_declaration final : public declaration
    {
    public:
        struct_declaration(identifier name, const scope* parent_scope, bool is_intrinsic);
        virtual ~struct_declaration() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        struct_declaration(const struct_declaration& scope) = delete;
        struct_declaration(struct_declaration&& scope) = delete;
        struct_declaration& operator=(const struct_declaration& scope) = delete;
        struct_declaration& operator=(struct_declaration&& scope) = delete;
        
        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] object_const_shared_ptr index(const compilation_context& context, const identifier& name,
                                                          const source_information& source_info) const override;
        [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
                                                         std::vector<object_const_shared_ptr> compiled_args,
                                                         const source_information& source_info) const override;
        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                            const source_information& source_info) const override { return wrapper; }

        [[nodiscard]] bool serializable(const compilation_context& context) const override;
        [[nodiscard]] bool deserializable(const compilation_context& context) const override;
        [[nodiscard]] object_const_shared_ptr generate_placeholder(
            const compilation_context& context, int& placeholder_index) const override;

    private:
        std::unique_ptr<user_type> type;
    };

    class constraint_declaration final : public declaration
    {
    public:
        constraint_declaration(identifier name, const scope* parent_scope, bool is_intrinsic);
        virtual ~constraint_declaration() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        constraint_declaration(const constraint_declaration& scope) = delete;
        constraint_declaration(constraint_declaration&& scope) = delete;
        constraint_declaration& operator=(const constraint_declaration& scope) = delete;
        constraint_declaration& operator=(constraint_declaration&& scope) = delete;

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                            const source_information& source_info) const override { return wrapper; }

    private:
        std::unique_ptr<constraint> constraint_;
    };

    class function_declaration final : public declaration
    {
    public:
        function_declaration(identifier name, const scope* parent_scope, bool is_intrinsic);
        virtual ~function_declaration() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        function_declaration(const function_declaration& scope) = delete;
        function_declaration(function_declaration&& scope) = delete;
        function_declaration& operator=(const function_declaration& scope) = delete;
        function_declaration& operator=(function_declaration&& scope) = delete;

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
                                                         std::vector<object_const_shared_ptr> compiled_args,
                                                         const source_information& source_info) const override;
        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                            const source_information& source_info) const override;

        [[nodiscard]] bool valid_at_boundary(const compilation_context& context) const;

        std::variant<std::unique_ptr<object>, const object*> body;

    private:
        std::unique_ptr<constraint> constraint_;
    };

    class namespace_declaration final : public declaration
    {
    public:
        namespace_declaration(identifier name, const scope* parent_scope);
        virtual ~namespace_declaration() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        namespace_declaration(const namespace_declaration& scope) = delete;
        namespace_declaration(namespace_declaration&& scope) = delete;
        namespace_declaration& operator=(const namespace_declaration& scope) = delete;
        namespace_declaration& operator=(namespace_declaration&& scope) = delete;

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth) const override;
        [[nodiscard]] object_const_shared_ptr index(const compilation_context& context, const identifier& name,
                                                          const source_information& source_info) const override;
        //todo: required because typeof does compilation, might need to change that?
        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                            const source_information& source_info) const override { return wrapper; }
    };
}
