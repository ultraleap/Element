Interfaces (a.k.a. typeclasses, traits) are collections of function constraints that can implemented for data types to explicitly implement a particular capability on a data type.

Example:
```
interface Equatable(type:struct)
{
    # one of these must be shadowed
    # or else their implementations will by cyclic
    eq(type, type):Bool = neq >> negate;
    neq(type, type):Bool = eq >> negate;
}

# Applied to a structure:
implement Equatable(num)
{
    neq(a:num, b:num):Bool = a.sub(b).abs.to(Bool);
} 
```
This example introduces the `struct` constraint as well as usage of constraints as values for interfaces.

Ideally a different way of specifying minimal implementations is possible.
The example taking advantage of cyclic references being illegal is potentially problematic if that restriction is relaxed in future.