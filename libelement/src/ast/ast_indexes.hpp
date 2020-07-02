#pragma once


namespace ast_idx
{

namespace function
{
    constexpr size_t declaration = 0;
    constexpr size_t body = 1;
}

namespace lambda
{
    constexpr size_t inputs = 0;
    constexpr size_t body = 1;
}

namespace declaration
{
    constexpr size_t inputs = 0;
    constexpr size_t outputs = 1;
}

namespace ns
{
    constexpr size_t body = 0;
}

namespace port
{
    constexpr size_t type = 0;
}

}