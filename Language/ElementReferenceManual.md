# Element Reference Manual
Element is a minimal embeddable programming language designed as a data format for writing algorithms.
Element code is purely functional and statically type checked.

Its primary goals are:
* **Portability**. Designed to be parsed, compiled and executed by many different hosting environments.
* **Simplicity**. Syntax and semantics are simple but practical - achieving portability is easy.
* **Visualizability**. Designed to interact with visual editing tools.
* **Guarantees**:
    * **Fixed memory usage**.
        * No dynamic allocation so memory usage is constant and calculable.
        * Note that memory management is implementation defined.
    * **No runtime errors**.
        * Compiled Element will run to completion and produce a result (assuming the host is well-formed and the function halts).
    * **Halting**. (Except when using `for` with non-constant exit condition)
        * Element is guaranteed to execute in a predictable time and take a fixed amount of resources.
        * `for` introduces dynamic loops and hence the halting problem.
    * **Pure Functions**.
        * Element is stateless and causes no side effects.
        The result is referential transparency: the same inputs will always provide the same outputs.

As an embeddable language, Element code requires a hosting environment to do anything useful with.
The job of a host is to provide capabilities for parsing, analysing and subsequently compiling or evaluating Element functions.

Element has multiple host implementations:

Name | Language | Evaluation | Compilation
---|---|---|---
Element.NET | C# | [LINQ.Expressions](https://docs.microsoft.com/en-us/dotnet/api/system.linq.expressions.expression) | Bytecode, C
libelement | C++ | Supported | -
PyElement | Python | [eval()](https://docs.python.org/3/library/functions.html#eval) | -



# Element Language 
## Lexical Conventions
Element code is expected to be encoded in UTF-8.

Element ignores whitespace, line endings and comments between tokens.

Comments are delimited by `#` until the end of line and are stripped during preprocessing.

Identifiers are used to name expressions, functions and types.
Identifiers can be any string of letters, digits and underscores, not starting with a digit and excluding reserved identifiers.
Identifiers are case-sensitive however reserved identifiers exclude mixed case, e.g. `intrinsic` is reserved thus `InTrInSiC` is considered invalid.

Reserved identifiers:
```
_
intrinsic
namespace
return
struct
```

`struct` identifiers are also reserved within their local scope.
Nested scopes in structs do not have this restriction and will shadow the constructor although doing so is not recommended.
```
struct MyStruct
{
    # Invalid, MyStruct is reserved
    MyStruct = 5
    namespace MySpace
    {
        # Valid but not recommended as this will shadow the MyStruct constructor
        MyStruct = 10
    }
}
```

## Syntax
A complete description of Element's syntax in .ebnf form can be found [here](../Common/Grammar.ebnf).
The ebnf does not include terminals for whitespace or comments.
Hosts may deal with these however they wish provided the language grammar remains intact.

The grammar is not an exact specification of valid Element code.
For example, reserved keywords are not mentioned in the grammar.
Hosts are expected to validate syntax elements after parsing to discover and report these errors.

## Basic Concepts
### Values
A value is the basic unit of an Element program.
Values can be declared using language constructs or returned as the result of expressions.
Values can have one or many of the following characteristics in Element:
* **Indexable** - can contain other values accessible via `.`, e.g. `foo` where `foo.bar` is valid.
* **Callable** - can produce other values through parametrization, e.g. `foo` where `foo(parameter)` is valid.
* **Constraint** - can accept or reject other values based on some predicate, e.g. `Any` which accepts any value or `Num` which accepts only numbers.
* **Serializable** - can be dealt with at the host boundary.

The kinds of values found in Element:

Name|Indexable|Callable|Constraint|Serializable|Example|Note
---|---|---|---|---|---|---
`Num`|Yes|-|Yes|Yes|`3.14159`|Number literals behave the same as `struct` instances and can be indexed to access `Num` instance functions
`Any`|-|-|Yes|-|-|Any is purely a Constraint and cannot have instances
Function constraint|-|-|Yes|-|`constraint Predicate(a):Bool`|^
`struct` declaration|Yes|Yes|Yes|-|`struct Vector3(x:Num, y:Num, z:Num)`|Calling a struct creates an instance
`struct` instance|Yes|-|-|Maybe|`a = Vector3(3, 6, 9)`|Can Index fields and instance functions, Serializable when all fields are, additionally List instances are Serializable when the Indexer only accesses Serializable elements
`namespace`|Yes|-|-|-|`namespace MySpace { ... }`|-
Constant function|-|-|-|Maybe|`tau = pi.mul(2)`|A function with no inputs and its result are equivalent - Serializability depends on the return value
Parameterized function|-|Yes|-|-|`sqr(a:Num):Num = a.mul(a)`|-
Lambda function|-|Yes|-|-|`_(_):List = 5`|-

### Expressions
Expressions are used build behaviour in Element.
Expressions are composable and can reference other values or contain many expressions.
An expression may be:
* a `Num` literal, e.g. `5`
* a call expression (a.k.a. function application), e.g. `add(1)`
* an indexing expression, e.g. `vector.x`
* a lambda, e.g. `_(a, b) = a.add(b)`
* an expression list involving many expressions, e.g. `radians.mul(180.div(Num.pi))`

The difference between an expression and a value is that expressions are not values themselves, they define/produce values.
Some expressions define values directly:  
* `5` defines a `Num` value
* `_(a, b) = a.add(b)` defines a function

Other expressions produce values:
* Call expressions compute new values based on input
* Indexing expressions reference existing values from elsewhere


### Identifiers & Bindings
An identifier is a name given to a value to allow referencing it in expressions.
Not all values have identifiers (e.g. a lambda), values without identifiers are only referenced in the location they are defined.

Binding is the process of assigning a value to an identifier using `=` to reuse in more complex expressions.

Identifiers are shorthand for their bound expression and are substituted for the expression during compilation (formally called beta reduction).
```
# binds x to a literal value of 5
x = 5

# binds y to the add function call's return value
y = x.add(1)

# binds z to an expression list involving nested expressions which converts radians to degrees
z = radians.mul(180.div(Num.pi))
```

Note that binding an expression does not directly cause a compiler to generate instructions for storing its value.
When performing analysis a host will optimize away all non-intrinsic bindings leaving only those which require storage instructions.
This is what makes Element declarative rather than imperative - bindings express to the compiler what is desired rather than exactly what to do.

## Numbers
`Num` is the only primitive data type in Element, representing real numbers.

`Num` values are:
* Indexable - `Num` has no fields but does have instance functions defined on the `struct` declaration for `Num`.
* Constraints - `Num` is a concrete Constraint which can be used to require that a value be a number.
* Serializable - numbers are plain data and can be serialized across the host boundary.

Numbers are definable as literals or returned as a result from a `Num` expression.
Literal number values can be declared in the following forms:
```
integer notation:     0        5        -10        +15
rational notation:    0.0      5.2      -10.86     +3.14159
e notation:           1e0      3E-5     -8e7       +2.998E8

a = 1e5
b = a.add(+10.3).mul(-5)
```
Many intrinsic functions are constrained to operate on numbers:
```
add(a:Num, b:Num):Num # Performs addition
sub(a:Num, b:Num):Num # Performs subtraction
div(a:Num, b:Num):Num # Performs division
mul(a:Num, b:Num):Num # Performs multiplication
```

## Namespaces
A namespace is a value containing other values which can be accessed via Indexing.
```
namespace Foo
{
    x = 10
}

# Indexes into Foo to retrieve x, resolving to 10
a = Foo.x
```

## Identifier Resolution & Indexing
Identifiers refer to the value they are bound to.
When referencing an identifier, the value is resolved using identifier resolution rules.

Identifiers are defined within scopes - scopes are nestable collections of identifiers defined by several language constructs such as namespaces.
Expressions can access identifiers defined in outer scopes implicitly.
The root scope is known as the global scope as its identifiers can be accessed from anywhere.

Identifier resolution rules are as follows:
1. Local identifiers in scope containing the expression being resolved
2. Function arguments when in a function's scope
3. Outer scoped identifiers, repeating 1 and 2 recursively until global scope

Identifier resolution is distinct from Indexing.
Indexing is an expression `.` for accessing contents of a value - values have their own Indexing rules.

Note that this is distinct from many other languages which have scoped identifiers.
In Element, `Foo.Bar` resolves `Foo` using identifier resolution and `.Bar` by Indexing the value identified by `Foo`.
If `Foo` fails to identify a value, the expression is invalid.
```
x = 5
namespace Foo
{
    y = 10
    namespace Bar
    {
        x = 15
    }

    # Does not identify a local x within Foo, recurses to global scope and finds x, resolving to 5
    a = x
    
    # Immediately identifies the Bar defined in Foo, indexing x within it, resolving to 15
    b = Bar.x

    # Error, indentifies local Bar but indexing a nested y within fails
    c = Bar.y

    # Identifies y locally, resolving to 10
    d = y

    # No local Foo, recurses to the global scope and identifies Foo, indexing it to find y, resolving to 10
    e = Foo.y
}
```

### Single Static Assignment (SSA)
Element uses single static assignment (SSA): identifiers are immutable and unique in each scope.
If `x` is defined as `5`, it cannot be rebound to `6` within the same scope.
A consequence of this is that the order expressions are defined doesn't matter.
SSA does not disallow shadowing, the same identifier may be defined in different scopes.
```
x = 5
namespace Outer
{
    # Refers to the global x since x is not defined in Outer
    y = x.add(5)
    namespace Inner
    {
        # Refers to global x and Outer.y
        # Shadows z defined in Outer, references to z in this scope or nested scopes will use this z
        z = x.add(y)
    }

    # Valid as global x and Outer.x are unique
    x = 5

    # This is invalid, y is already defined in Outer
    y = 10

    # Valid as Outer.z and Outer.Inner.z are unique
    z = x.add(y)
}
```
Since all bindings are immutable we deliberately avoid using the term `variable` to describe them.

### Cyclic Expressions
It is syntactically possible to define cyclic expressions that reference themselves since definition order doesn't matter.
Expressions must be acyclic, cyclic expressions (directly or indirectly caused) are illegal.
For explicit looping, the `for` intrinsic is available.
```
# Directly cyclic expression - a depends on itself, causing an infinite cycle
a = a.add(1)

# Indirectly cyclic expressions - b and c depend on each other, causing an infinite cycle
b = c.add(1)
c = b.add(1)
```

## Functions
Functions are Callable values which return a value when parameterized by one or more other expressions.
Functions are syntactically defined by 2 components, their interface (or declaration) and their function body.

A function's interface includes a parameter list and a return type.
The function body defines one or many expressions which relate the parameters to the return value.
```
# degrees is a function that coverts radians to degrees
degrees(radians) = radians.mul(180.div(Num.pi))

# lerp is a function that performs linear interpolation, returning t amount along a to b
lerp(t, a, b) = a.add(t.mul(b.sub(a)))
```

Calling functions binds the resulting value, these locations are known as call sites.
```
# binds deg to pi radians, resulting in 180
deg = degrees(Num.pi)
```

Functions can define a scope to declare intermediate expressions for calculating the result.
In a function scope the result must be bound to a `return` identifier.
```
# mod is the Modulo function, returning the remainder rounded towards negative infinity
mod(a, b)
{
    # c is referenced multiple times in the return expression
    # thus it is helpful to define it once instead of repeating
    c = a.rem(b)
    return = c.mul(b).lt(0).if(add(c, b), c)
}
```

Function scopes cannot be Indexed externally, their values are hidden as implementation details.
The only way to return a value from a function is via its result.
```
# Error, mod is a function which can only be called or referenced in an expression
a = mod.c

# Error, calling mod results in a number which does not contain value c when indexed
b = 5.mod(4).c
```

As with other identifiers, function identifiers must be unique, function overloading is disallowed.
```
# Valid, foo is bound and identifies a function with 2 parameters
foo(a, b) = ...

# Error, foo is already a bound identifier in this scope
foo(a, b, c) = ...
```

### High Order Functions
Functions are first-class values and can be passed into and returned from other functions like any other value.
```
# Calls the input function twice on the input value
applyTwice(function, value) = function(function(value))

# Returns a function defined locally within another function
makeAdder(amountToAdd)
{
    return(a, amountToAdd) = a.add(amountToAdd)
}

# Partial application - binds halfAlong to 0.5 way between min and max
halfAlong(min, max) = lerp(0.5, min, max)
# resolves a to 15
a = halfAlong(10, 20)
```

### Recursion
Since cyclic references are illegal, recursion is also disallowed.
A function may not reference itself directly or indirectly.
Iteration is available as an alternative via the `for` intrinsic.
```
factorial(a)
{
    body(t) = Tuple(t.item1.add(1), v.item2.mul(v.item1))
    return = for(Tuple(1, 1), _(t) = leq(t.item1, a), body).item2
}
```

### Captures
A function scope can contain any other values, e.g. other functions.
```
scaleAndSumNumbers(a, b, c, scale)
{
    # doScale is defined only within scaleAndSumNumbers
    doScale(number) = number.mul(scale)
    sa = doScale(a)
    sb = doScale(b)
    sc = doScale(c)
    return = sa.add(sb.add(sc))
}
```

When defining a nested value within a function, the nested value may depend on values which are part of the outer function.
When these nested values directly or indirectly depend on input of the outer function then they are "captured" when the nested value is passed around or returned.
```
# value is captured by the Indexer in the returned List
repeat(value, count) = List(_(_) = value, count)

# the return type of myFunction is "struct Voldemort"
# but this type is inaccessible from outside of myFunction
# Voldemort's struct declaration is captured by type instance
myFunction(a, b)
{
    struct Voldemort(u, v)
    return = Voldemort(a, b)
}
```

### Unidentifier
It is sometimes useful to avoid binding an identifier for an expression when it is unused or only required once.
In these cases the unidentifier `_` can be used:

Lambdas (anonymous/unidentified functions) - relevant when defining single-use functions within another expression
```
# binds an unidentified function to the last argument of fold
# sum is all array elements added together
sum = array(1, 2, 3).fold(0, _(accum, element) = accum.add(element))

# binds the lambda as the return value of the function
# equivalent to the makeAdder function in the High Order Functions example above
makeAdder(amountToAdd) = _(a) = a.add(amountToAdd)
```
Unused arguments - relevant when a function's interface demands arguments that are unused in the body
```
repeat(value, count)
{
    # For any index it returns the same value.
    # _ is used to intentionally ignore the index parameter
    index(_) = value
    return = List(index, count)
}
```

### Type Annotations
Element is statically typed; all expressions have an associated type which is checked for consistency when compiling.
Types are Constraints used to limit what values a function can accept and return.
Types are declared using `:` after a parameter's identifier or, for return types, after the function parameter list.

Type annotations are optional - when no type is given, `Any` is implied.
```
# to calls the given Unary function to turn a Num into any other type
to(a:Num, constructor:Unary) = constructor(a)
```

### Higher Order Types
Element explicitly doesn't support constraints as parameters or return types.
This would allow parameterized types - the implications of which have not been fully considered.
```
List(t)
{
    tIndexer(idx:Num):t
    struct return(at:tIndexer, count:Num)
}

sum(a:List(Num)) = a.fold(0, (accum, element) = accum.add(element))
```
A [proposal](../Language/Proposals/TypeFunctions.md) exists for allowing this behaviour.

## Constraints
Constraints are expressions used to limit what kinds of values can be dealt with.
Constraints are used as type annotations for Callable values and other Constraints.

### Concreteness
Constraints can be concrete or non-concrete (polymorphic).

A concrete constraint is one which matches only one type of value, e.g. `Num` or `struct Vector3(x:Num, y:Num, z:Num)`.

Non-concrete constraints are polymorphic - they allow more generalized expressions where many types of values are interchangeable, e.g. `Any` or `Predicate(a):Bool`

### Any
The `Any` constraint allows any value to be dealt with and can be thought of as the absence of a constraint.
Since this constraint is guaranteed to pass, it is possible to write generic code where the passed value's constraint is an implicit interface of all the locations the value is referenced.
```
# a is of type any
# call sites of even must pass a value which can Index an instance function rem(a, b:Num)
# which in turn must return a value which can Index an instance function eq(a, b:Num):Bool
# in order to pass type checking
even(a):Bool = a.rem(2).eq(0)

# Changing a to type Num requires that a be of exactly type Num
# This is more explicit but less flexible as there could be other types which can satisfy type checking of the generic version
even(a:Num):Bool = a.rem(2).eq(0)
```

### Function Constraint (a.k.a. Function Type or Function Interface)
Function constraints allow constraining the type of a expression to a function with specific parameters and return.
```
# Predicate is a function type taking a parameter of any type and returning a bool
Predicate(a):Bool

# Any checks if any list elements pass the given predicate
any(list:List, pred:Predicate):Bool
    = list.fold(Bool.false, _(accum, element) = accum.or(pred(element)))

# The interface of even matches Predicate
even(a:Num):Bool = a.rem(2).eq(0)

# Thus we can call any, passing the function even as an instance of Predicate
anyEven = any(array(1, 3, 5), even)
# anyEven evaluates to Bool.false since none of the array elements are even
```

## Structures
Structures are a value grouping mechanism.
Structures involve 2 distinct kinds of values, `struct` declarations and instances.

### Structure Declarations
Structures are defined using the `struct` qualifier followed by a parameter list defining the fields that a `struct` instance must have.  
```
struct Complex(real:Num, imaginary:Num)
```
A fields is simply a value in a structures group.
`real` and `imaginary` are the fields for a `Complex` instance.

`struct` declaration values are:
* Callable - as a constructor function for creating `struct` instances.
* Indexable - `struct` declarations can contain other values in an optional scope.
* Constraints - `struct` declarations can be used to limit values to instances of the `struct`.

Constructors are automatically implemented by the compiler.
```
# Creates a Complex instance with real number 5 and imaginary number 10
myComplex = Complex(5, 10)
```

Note that the `struct` declaration's identifier is reserved within the structures scope (but not nested ones).
```
struct Complex(real:Num, imaginary:Num)
{
    # adds two complex numbers together, resulting in another complex
    # the implementation must use the Complex constructor to create the result
    add(a:Complex, b:Complex):Complex = Complex(a.real.add(b.real), a.imaginary.add(b.imaginary))
    
    # invalid, Complex is reserved within the struct scope
    Complex = 5
    
    namespace Foo
    {
        # valid but strongly discouraged as this will shadow the outer struct
        # compilers will issue a warning when this shadowing occurs
        Complex = 5
    }
}

# Usage example, a and b are struct instances
a = Complex(5, 5)
b = Complex(8, 8)

# Add can be called via indexing Complex
c = Complex.add(a, b)
```

### Structure Instances
A `struct` instance is a value with a layout conforming to a specific `struct` declaration.

`struct` instances are:
* Indexable - provides access to struct members - members are fields and instance functions.
* Serializable (sometimes) - when all fields are serializable, the instance is also serializable.

#### Instance Functions
Instance functions are functions defined within a `struct` declaration's scope where:
* The first argument is the `struct` declaration's type.
* Instance functions are Indexable from `struct` instances.
* Instance functions apply the first parameter as the `struct` instance being Indexed.
```
# continuing from the example above...
# add can be called as an instance function since it's first parameter is type Complex
# d is an equivalent expression to c above
d = a.add(b)
```

Instance function calls can be chained allowing natural expressions which read left to right.
```
# binds numbers to an array of all even numbers n..m divisible by 5.
# List.range is prefix called (it is a generator and can't be infix called)
# and then transformed by infix calling filter
# even is a predicate returning 1 or 0 if a number is even or not
numbers = List.range(n, m)).filter(_(i) = even(i).and(i.rem(5).eq(0)))
```

## Intrinsics
Intrinsics are functions and types which hosts must explicitly support.
All Element programs are composed from intrinsics to perform tasks.
Intrinsics can be declared in source code using the `intrinsic` keyword.

Intrinsics have a few special rules that differ from normal structs and functions:
* Intrinsic functions are implemented by hosts.
* Intrinsics can be variadic (types or functions with variable number of fields).
Element does not directly support variadic functions or types however a few intrinsic constructs are variadic and can be referenced as long as their interfaces are satisfied.
* Intrinsics may omit an interface as it may not be possible to describe in the language (due to allowing variadics).

# Prelude
Prelude is a base library of Element source code containing intrinsics and values implementing core functionality.
The source for Prelude can be found [here](../Common/Prelude/Prelude.ele).

### Bool
`Bool` is an intrinsic type implemented in the Prelude.
`Bool` is not a primitive type, it is backed by a `Num`.
When creating an instance of `Bool` using the constructor the compiler must refine the backing `Num` to 1 or 0, signifying `True` and `False` respectively.
The formula for refinement is `value > 0 ? 1 : 0`, i.e. positive numbers are truthy, negative numbers and 0 are falsy.

### List
`List` is an intrinsic type implemented in the Prelude.
`List(at:Indexer, count:Num)` is defined as an indexing function `constraint Indexer(i:Num)` and a numerical count.
`List` has no special compiler support except that instances can be serializable when the return type of `at:Indexer` is serializable.
`List` serialization is performed by evaluating all elements.

# Host Environments
An Element program exists in context of a hosting environment (host) for dealing with it.
Hosts are libraries or programs providing facilities for parsing, validating, analysing, compiling and evaluating Element source directly (interpreting) or transforming them into another format for export or transport.

## Top-level Functions
An Element program is always a function (referred to as the top-level function) that takes inputs and returns outputs when the program is run.
There is no notion of a "main" function - any function that meets data boundary criteria may be compiled by a host.
Top-level functions must only have serializable values on their interface.

## Compilation
Compiling involves starting with the outputs and recursively analysing all dependencies.
Resolving type constraints statically relies on recursive compilation of dependencies.
Output values necessarily have concrete types that are checked against constraints as the dependency graph is traversed.
All constraints can be checked during compilation with no type checking requiring deference until runtime.

Another important consequence of recursive analysis is that unreferenced values are not compiled.
This is a double-edged sword as it improves compilation performance but allows errors in unreferenced code that parses successfully (such as type errors) to go undetected!

Hosts must follow the error handling conventions defined in [Laboratory's readme](../Laboratory/README.md).

## Data Boundary
Element has no facilities for dealing with storage/memory thus a host must be responsible for handling memory for an Element program.
Boundaries occur when an Element program relies on the host for handling memory.
This happens in two situations:
* when passing the inputs to and receiving outputs from the function being compiled
* in the iteration state values of `persist` and `for` intrinsic calls.

### Serialization
Only serializable values can be passed across the boundary.
Serializable values are those which can be translated into a sequence of primitive data - numbers in Elements case.
The following value types are serializable:
* `Num`
* `struct` instance containing only serializable fields - structures require mapping between host and Element types to match layout.
* `List` which can be evaluated to only serializable elements - Lists will be evaluated to their elements when crossing the boundary.

Hosts must have member mappings for `struct` types declared in the prelude and may provide mappings for other structures.

All concrete types in a `List` must be mappable for a `List` to be translatable.