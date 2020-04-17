#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

typedef size_t element_type_id;

#define DECLARE_TYPE_ID() static const element_type_id type_id
#define DEFINE_TYPE_ID(cls, n) const element_type_id cls::type_id = n

template <typename T>
struct rtti_type
{
public:
    element_type_id subtype() const { return m_type_id; }

    template <typename TD>
    bool is() const
    {
        return std::is_base_of<T, TD>::value && (subtype() & TD::type_id) != 0;
    }

    template <typename TD>
    TD* as()
    {
        return is<TD>() ? static_cast<TD*>(this) : nullptr;
    }

    template <typename TD>
    const TD* as() const
    {
        return is<TD>() ? static_cast<const TD*>(this) : nullptr;
    }

protected:
    rtti_type(element_type_id id) : m_type_id(id) {}

private:
    const element_type_id m_type_id;
};


struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const
    {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};
