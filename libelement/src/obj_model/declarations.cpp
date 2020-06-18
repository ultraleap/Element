#include "declarations.hpp"

//SELF
#include "object.hpp"
#include "scope.hpp"
#include "expressions.hpp"
#include "intermediaries.hpp"
#include "functions.hpp"
#include "etree/expressions.hpp"

namespace element
{
    //declaration
    declaration::declaration(identifier name)
        : declaration(std::move(name), nullptr)
    {
    }

    declaration::declaration(identifier name, const scope* parent)
        : name(std::move(name))
        , our_scope(std::make_unique<scope>(parent, this))
    {
    }

    bool declaration::has_scope() const
    {
        return our_scope && !our_scope->is_empty();
    }

    std::string declaration::location() const
    {
        auto declaration = name.value;

        if (!our_scope)
            return declaration;
        
        if (!our_scope->get_parent_scope() || our_scope->get_parent_scope()->is_root())
            return declaration;

        //recursive construction
        return our_scope->get_parent_scope()->location() + "." + declaration;
    }

    //struct
    struct_declaration::struct_declaration(identifier identifier, const scope* parent_scope, const bool is_intrinsic)
        : declaration(std::move(identifier), parent_scope)
    {
        qualifier = struct_qualifier;
    }

    std::string struct_declaration::to_string() const
    {
        return location() + ":Struct";
    }

    std::string struct_declaration::to_code(const int depth) const
    {
        std::string ports;

        const std::string offset = "    ";
        std::string declaration_offset;

        for (auto i = 0; i < depth; ++i)
            declaration_offset += offset;

        if (has_inputs()) {
            static auto accumulate = [depth](std::string accumulator, const port& port)
            {
                return std::move(accumulator) + ", " + port.to_string() + port.to_code(depth);
            };

            const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].to_string() + inputs[0].to_code(depth), accumulate);
            ports = "(" + input_ports + ")";
        }

        if (_intrinsic)
            return declaration_offset + "intrinsic struct " + name.value + ports + our_scope->to_code(depth);

        return declaration_offset + "struct " + name.value + ports + our_scope->to_code(depth);
    }

    std::shared_ptr<object> struct_declaration::index(const compilation_context& context, const identifier& identifier) const
    {
        return our_scope->find(identifier.value, false);
    }

    //constraint
    constraint_declaration::constraint_declaration(identifier identifier, const bool is_intrinsic)
        : declaration(std::move(identifier))
    {
        qualifier = constraint_qualifier;
        _intrinsic = is_intrinsic;
    }

    std::string constraint_declaration::to_string() const
    {
        return location() + ":Constraint";
    }

    std::string constraint_declaration::to_code(const int depth) const
    {
        std::string ports;

        const std::string offset = "    ";
        std::string declaration_offset;

        for (auto i = 0; i < depth; ++i)
            declaration_offset += offset;

        if (has_inputs()) {
            static auto accumulate = [depth](std::string accumulator, const port& port)
            {
                return std::move(accumulator) + ", " + port.to_string() + port.to_code(depth);
            };

            const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].to_string() + inputs[0].to_code(depth), accumulate);
            ports = "(" + input_ports + ")";
        }

        if (output)
            ports += output->to_code(depth);

        if (_intrinsic)
            return declaration_offset + "intrinsic constraint " + name.value + ports;

        return declaration_offset + "constraint " + name.value + ports;
    }

    //function
    function_declaration::function_declaration(identifier identifier, const scope* parent_scope, const bool is_intrinsic)
        : declaration(std::move(identifier)
        , parent_scope)
    {
        qualifier = function_qualifier;
        _intrinsic = is_intrinsic;
    }

    std::string function_declaration::to_string() const
    {
        return location() + ":Function";
    }

    std::string function_declaration::to_code(int depth) const
    {
        auto declaration = name.value;
        std::string ports;

        const std::string offset = "    ";
        std::string declaration_offset;

        for (auto i = 0; i < depth; ++i)
            declaration_offset += offset;

        if (has_inputs()) {
            static auto accumulate = [depth](std::string accumulator, const port& port)
            {
                return std::move(accumulator) + ", " + port.to_string() + port.to_code(depth);
            };

            const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].to_string() + inputs[0].to_code(depth), accumulate);
            ports = "(" + input_ports + ")";
        }

        if (output)
            ports += output->to_code(depth);

        if (_intrinsic)
            return declaration_offset + "intrinsic " + name.value + ports + ";";

        return declaration_offset + name.value + ports + our_scope->to_code(depth);
    }

    std::shared_ptr<object> function_declaration::index(const compilation_context& context, const identifier& name) const
    {
        //Indexing a nullary(constant is valid), implicit call and then index
        //e.g. Num.pi.add(1)
        //if it's not a nullary, then the call will fail
        return call(context, {})->index(context, name);
    }

    std::shared_ptr<object> function_declaration::call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const
    {
        /**
        todo: write tests
         MyStruct(myNum:Num)
         {
            DoThing(this:MyStruct, a:Num) = a.mul(this.myNum);
         }

         MyStruct.DoThing(MyStruct(1), 1)
         MyStruct(1).DoThing(1)
         
         myStructInstance = MyStruct(5);
         returnHigherOrder = myStructInstance.DoThing;
         useHigherOrderFunc(a:Unary) = a;
         useHigherOrderFunc(useHigherOrderFunc(returnHigherOrder));
         eval = a(2)
         
         find DoThing(), not a member, so check struct declaration
         if DoThing is instance function
            return dothing->call(shared_from_this())
         
         */

        //Tried to call a function declaration without passing the right number of arguments
        //If this was via an instance function, the struct instance will inject itself in to the arguments, so not something we concern ourselves with
        //Element itself doesn't support partial application of any function
        if (inputs.size() != args.size())
        {
            assert(false);
            return nullptr;
        }

        //If we're scope-bodied compiling ourselves means return the return in our scope
        //    if that return is a nullary, we compile it and return
        //    if that return is anything else, we return it as a higher-order function
        //If we're expression-bodied compiling ourselves means compiling the expression chain
        //    if they use our parameters, they should get it from the callstack
        //If we're intrinsic-bodied compiling ourselves means generating a compiled_expression representing an expression tree
        //    intrinsics require parameters, which they should get it from the callstack

        call_stack::frame frame;
        frame.function = this;
        frame.arguments = std::move(args);
        context.stack.frames.emplace_back(std::move(frame));
        auto ret = body->call(context, {});
        context.stack.frames.pop_back();
        return ret;
    }

    std::shared_ptr<compiled_expression> function_declaration::compile(const compilation_context& context) const
    {
        //E.g. evaluate = Num.pi;
        
        //Nullaries can always be compiled without making a function instance
        if (inputs.empty())
        {
            auto obj = call(context, {});
            assert(dynamic_cast<compiled_expression*>(obj.get())); //todo: one of the calls returned something that wasn't wrapped in a compiled expression
            return std::dynamic_pointer_cast<compiled_expression>(std::move(obj));
        }

        //Compiling a declaration that isn't nullary creates a function instance, that can later have stuff applied to it
        //This allows it to keep a copy of the callstack for when it is called later on
        auto ret = std::make_shared<compiled_expression>();
        ret->creator = this;
        ret->type = nullptr;
        ret->expression_tree = nullptr;
        auto instance = std::make_shared<function_instance>(this, std::vector<std::shared_ptr<compiled_expression>>{});
        instance->stack = context.stack;
        ret->object_model = std::move(instance);
        return ret;
    }

    //namespace
    namespace_declaration::namespace_declaration(identifier identifier, const scope* parent_scope)
        : declaration(std::move(identifier), parent_scope)
    {
        qualifier = namespace_qualifier;
        _intrinsic = false;
    }

    std::string namespace_declaration::to_string() const
    {
        return location() + ":Namespace";
    }

    std::string namespace_declaration::to_code(const int depth) const
    {
        return "namespace " + name.value + our_scope->to_code(depth);
    }

    std::shared_ptr<object> namespace_declaration::index(const compilation_context& context, const element::identifier& expr) const
    {
        return our_scope->find(name.value, false);
    }
}
