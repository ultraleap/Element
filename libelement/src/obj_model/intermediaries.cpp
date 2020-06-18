#include "intermediaries.hpp"

#include <cassert>

namespace element
{
    //compiled_expression
    std::shared_ptr<object> compiled_expression::index(const compilation_context& context, const identifier& identifier) const
    {
        if (object_model)
            return object_model->index(context, identifier);

        if (type)
        {
            //TODO: THIS IS HORRIBLE, FIX
            const auto type_type = context.get_global_scope()->find(type->get_name(), true);
            return type_type->index(context, identifier);
        }

        //todo: is there any point?
        if (creator)
        {
            assert(false); //figure out if/when this occurs and if it should
            return creator->index(context, identifier);
        }

        return nullptr;
    }

    std::shared_ptr<object> compiled_expression::call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const
    {
        if (object_model)
            return object_model->call(context, std::move(args));

        //todo: is there any point?
        if (creator)
        {
            assert(false); //figure out if/when this occurs and if it should
            return creator->call(context, std::move(args));
        }

        return nullptr;
    }

    std::shared_ptr<compiled_expression> compiled_expression::compile(const compilation_context& context) const
    {
        if (expression_tree)
            return const_cast<compiled_expression*>(this)->shared_from_this();

        if (object_model)
            return object_model->compile(context);

        if (creator)
        {
            assert(false);
            return creator->compile(context);
        }

        return nullptr;
    }

    //struct_instance
    struct_instance::struct_instance(const struct_declaration* declarer, const std::vector<std::shared_ptr<compiled_expression>>& expressions)
        : declarer{ declarer }
    {
        //TODO: JM - variadics
        assert(declarer->inputs.size() == expressions.size());
        for (size_t i = 0; i < declarer->inputs.size(); ++i)
        {
            //fields.emplace(declarer->inputs[i].identifier, expressions[i]);
        }
    }

    std::shared_ptr<object> struct_instance::index(const compilation_context& context, const identifier& identifier) const
    {
        //this is how we do partial application. if we index a struct instance and find it's an instance function
        //then we create a function_instance of that function, with ourselves as the first provided argument
        //when we return that function_instance, either the next expression is a call which fills the remaining arguments and then calls it
        //or we just return it/store it, to be used later
        return nullptr;
    }

    std::string struct_instance::to_string() const
    {
        return "Instance:" + declarer->to_string();
    }

    //function_instance
    function_instance::function_instance(const function_declaration* declarer, std::vector<std::shared_ptr<compiled_expression>> args)
        : declarer{ declarer }
        , provided_arguments(std::move(args))
    {
    }

    std::string function_instance::to_string() const
    {
        return declarer->location() + ":FunctionInstance";
    }

    std::shared_ptr<object> function_instance::call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const
    {
        args.insert(args.begin(), provided_arguments.begin(), provided_arguments.end());

        if (args.size() == declarer->inputs.size())
        {
            /*
             someFunc()
             {
                doFunc(a) = a.add(1);
                otherFunc = Num.add(1, 2);
                return = doFunc(otherFunc);
             }

             evaluate = someFunc;
             */

            //we have the exact arguments we need, so now we just need to perform the call
            //we also need to swap the callstacks so that the call can use the one at the point the instance was created
            std::swap(stack, context.stack);
            auto ret = declarer->body->call(context, args);
            //we then swap them back, in case our instance is called multiple times.
            //Technically the call we did could have modified the callstack, but if it's behaving correctly it will be identical due to popping frames
            std::swap(stack, context.stack);
            return ret;
        }

        assert(false);
        return nullptr;

        /* Element doesn't allow general partial application, but if it did, we would do it here
        if (args.size() < declarer->inputs.size())
        {
            //we partially apply the arguments we have. todo: reuse existing instance if we can?
            return std::make_shared<function_instance>(declarer, std::move(args));
        }
        */
    }

    std::shared_ptr<compiled_expression> function_instance::compile(const compilation_context& context) const
    {
        //todo: maybe create the wrapped instance of ourselves, ourselves, instead of relying on the function declaration to do it
        std::swap(stack, context.stack);
        auto ret = declarer->compile(context);
        //we then swap them back just in case;
        std::swap(stack, context.stack);
        return ret;
    }
}