#pragma once


namespace ast_idx
{

namespace fn
{
    constexpr size_t declaration = 0;
    constexpr size_t body = 1;
}

namespace lambda
{
    constexpr size_t inputs = 0;
    constexpr size_t body = 1;
}

namespace decl
{
    constexpr size_t inputs = 0;
    constexpr size_t outputs = 1;
}

namespace ns
{
    constexpr size_t body = 0;
}

namespace call
{
    constexpr size_t parent = 0;
    constexpr size_t args = 1;
}

namespace port
{
    constexpr size_t type = 0;
}

}