#include "for_wrapper.hpp"

//STD
#include <cassert>
#include <algorithm>

//SELF
#include "object_model/error.hpp"
#include "object_model/constraints/constraint.hpp"
#include "etree/expressions.hpp"

using namespace element;

/*
constant for loop if all of the objects (initial, predicate, body) are constant
    reimplement the for loop in evaluator

else it's a dynamic for loop

if to_expression fails on initial, error message, since it needs to be serializable
if predicate or body take inputs that are not serializable, error message, since they're almost
the same as a boundary function (except the return, we know it's the same type as initial)

if initial is an expression, create for instruction

else initial is an object, e.g. struct instance
    create for wrapper, so that when we index it, e.g. Vector2(0, 0).x, we create an
    indexing expression with the for loop as a child, or we need to make another for
    wrapper if the indexed thing is not a number (e.g. another struct instance)
 */
object_const_shared_ptr create_or_optimise(const object_const_shared_ptr& initial_object,
                                           const object_const_shared_ptr& predicate_object,
                                           const object_const_shared_ptr& body_object,
                                           const source_information& source_info)
{
    /*auto initial_error = std::dynamic_pointer_cast<const error>(initial_object);
    if (initial_error)
        return initial_error;

    auto predicate_error = std::dynamic_pointer_cast<const error>(predicate_object);
    if (predicate_error)
        return predicate_error;

    auto body_error = std::dynamic_pointer_cast<const error>(body_object);
    if (body_error)
        return body_error;
        
    const auto initial_expression = std::dynamic_pointer_cast<const element_expression>(initial_object);
    if (!initial_expression)
    {
        return std::make_shared<const error>(
            "Initial element must be a value type, functions are not valid as the initial parameter",
            ELEMENT_ERROR_UNKNOWN, source_info);
    }
    */


    //<IT'S CODING TIME>

    //constant for loop if all of the objects (initial, predicate, body) are constant
    const auto is_constant = initial_object->is_constant()
        && predicate_object->is_constant()
        && body_object->is_constant();

    if (is_constant)
    {
        
        return nullptr;
    }

 

    //else it's a dynamic for loop


    //if to_expression fails on initial, error message, since it needs to be serializable
    //if predicate or body take inputs that are not serializable, error message, since they're almost
    //the same as a boundary function (except the return, we know it's the same type as initial)


    //if initial is an expression, create for instruction


    //else initial is an object, e.g. struct instance
    //    create for wrapper, so that when we index it, e.g. Vector2(0, 0).x, we create an
    //    indexing expression with the for loop as a child, or we need to make another for
    //    wrapper if the indexed thing is not a number (e.g. another struct instance)

    return std::make_shared<const for_wrapper>(initial_object, predicate_object, body_object);

    //</IT'S CODING TIME>
}

for_wrapper::for_wrapper(const std::shared_ptr<const element_expression>& initial_object,
                         const std::shared_ptr<const element_expression>& predicate_object,
                         const std::shared_ptr<const element_expression>& body_object)
    : initial(initial_object)
    , predicate(predicate_object)
    , body(body_object)
{
    
}

std::string for_wrapper::typeof_info() const
{
    return "?";
}

std::string for_wrapper::to_code(int depth) const
{
    return "?";
}

bool for_wrapper::matches_constraint(const compilation_context& context, const constraint* constraint) const
{
    return false;
}

const constraint* for_wrapper::get_constraint() const
{
    return false;
}

object_const_shared_ptr for_wrapper::call(const compilation_context& context,
                                          std::vector<object_const_shared_ptr> compiled_args,
                                          const source_information& source_info) const
{
    return nullptr;
}

object_const_shared_ptr for_wrapper::index(const compilation_context& context,
                                           const identifier& name,
                                           const source_information& source_info) const
{
    /*

        if the result of for (same type as initial) is a struct (which it must be for the for wrapper to exist) then:
            grab that type and index it
                if we found something, just return it. e.g. Vector2(0, 0).magnitude will create the correct object that contains the for expression in its resulting expression tree somewhere
                if we didn't find something, then we need to check if it's a field
                    if it is a field and it's a value like Num or Bool (expressions, e.g. Vector2(0, 0).x), then we create an indexing expression, with the for loop as the 1st child and the index 0 as the second child
                    if it is a field and it's a non-value like a finger struct within a hand struct, then

struct Finger(x, y, z) {}
struct Hand(thumb) {}

makeHand(a:Num) = Hand(Finger(a, a, a));
predicate(hand:Hand) = 
evaluate(a:Num) = for(makeHand(a), predicate, body);


     */

    return nullptr;
}

object_const_shared_ptr for_wrapper::compile(const compilation_context& context,
                                             const source_information& source_info) const
{
    return nullptr;
}

std::shared_ptr<const element_expression> for_wrapper::to_expression() const
{
    return nullptr;
}