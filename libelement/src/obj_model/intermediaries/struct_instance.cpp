#include "struct_instance.hpp"

element::struct_instance::struct_instance(const element::struct_declaration* declarer, const std::vector<std::shared_ptr<expression>>& expressions)
    : declarer{ declarer }
{
    //TODO: JM - variadics
    assert(declarer->inputs.size() == expressions.size());
    for (size_t i = 0; i < declarer->inputs.size(); ++i)
    {
        fields.emplace(declarer->inputs[i].identifier, expressions[i]);
    }
}

std::string element::scope::to_string() const
{
    return "Instance:" + declarer->to_string();
}
