#include "struct_instance.hpp"

#include <cassert>

//compiled_expression
std::shared_ptr<element::element_object> element::compiled_expression::index(const indexing_expression* expr) const
{
    return object->index(expr);
}

//struct_instance
element::struct_instance::struct_instance(const struct_declaration* declarer, const std::vector<std::shared_ptr<expression>>& expressions)
    : declarer{ declarer }
{
    //TODO: JM - variadics
    assert(declarer->inputs.size() == expressions.size());
    for (size_t i = 0; i < declarer->inputs.size(); ++i)
    {
        //fields.emplace(declarer->inputs[i].identifier, expressions[i]);
    }
}

std::shared_ptr<element::element_object> element::struct_instance::index(const indexing_expression* expression) const
{
    //this is how we do partial application. if we index a struct instance and find it's an instance function
    //then we create a function_instance of that function, with ourselves as the first provided argument
    //when we return that function_instance, either the next expression is a call which fills the remaining arguments and then calls it
    //or we just return it/store it, to be used later
    return nullptr;
}

std::string element::struct_instance::to_string() const
{
    return "Instance:" + declarer->to_string();
}

//function_instance
element::function_instance::function_instance(const function_declaration* declarer, const std::vector<std::shared_ptr<expression>>& expressions)
    : declarer{declarer}
{
    //TODO: JM - variadics
    assert(declarer->inputs.size() == expressions.size());
    for (size_t i = 0; i < declarer->inputs.size(); ++i)
    {
        //provided_arguments.emplace(declarer->inputs[i].identifier, expressions[i]);
    }
}

std::shared_ptr<element::element_object> element::function_instance::call(const call_expression* expression) const
{
    //do things
    return nullptr;
}