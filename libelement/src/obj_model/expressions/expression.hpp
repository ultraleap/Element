#pragma once

#include "obj_model/element_object.hpp"

namespace element
{
    struct expression : public element_object
	{
    };

    //struct call_expression : public expression {
    //    explicit call_expression(element_object* parent) : expression(parent)
    //    {
    //    }
    //};
	
    //struct indexing_expression : public expression {
    //    explicit indexing_expression(element_object* parent) : expression(parent)
    //    {
    //    }
    //};
	
    //struct lambda_expression : public expression {
    //    explicit lambda_expression(element_object* parent) : expression(parent)
    //    {
    //    }
    //};
}
