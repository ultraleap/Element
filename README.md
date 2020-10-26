# Element
Element is a minimal functional programming language.
Element code runs using a host which can interpret or compile it to other formats.
One of these formats is the bytecode format, [LMNTIL (Element Intermediate Language)](LMNT), for use in native and embedded applications.

## Element Language
* Single number data type `Num`
* Structured types using `struct`
* First class functions and local functions
* Intrinsics (math intrinsics, collection intrinsics, control flow)
* Genericity via `Any` type

Example:
```
sum(list) = list.fold(0, Num.add);
factorial(n) = List.range(0, n).fold(1, Num.mul);
```

For full details see the [Element Reference Manual](Language/ElementReferenceManual.md).

### Element Libraries
Element includes a core set of functionality in the base library [Prelude](Common/Prelude).

Element also comes with a [Standard Library](Common/StandardLibrary).

## Repository Layout
* [Laboratory - The Element Host Test Suite](Laboratory) - .NET Core 3.0 CLI
    * Test runner using NUnit containing host compliance tests, prelude tests and standard library tests.
    * Can test Element.NET directly or invoke other compilers via CLI convention

### Host Libraries
* [Element.NET](Element.NET) - .NET Standard 2.0 ![](https://github.com/ultraleap/Element/workflows/Element.NET.yml/badge.svg)
    * Element parser using [Lexico](https://github.com/hamish-milne/Lexico)
    * Element function evaluation via:
        * Direct evaluation (slow)
        * Compilation to CLR Function using [LINQ Expressions](https://docs.microsoft.com/en-us/dotnet/api/system.linq.expressions.expression) (very fast)
    * AoT compilation targets including:
        * [LMNTIL](LMNT)
        * C
* [libelement](libelement) - C++
    * Element parser
    * Element function evaluation
    * AoT compilation to:
        * [LMNTIL](LMNT/doc/Bytecode.md) - planned
* [LMNT](LMNT) - C
    * [LMNTIL](LMNT/doc/Bytecode.md) interpreter
    * JIT compiler using [DynASM](https://luajit.org/dynasm.html)
        * x86_64
        * ARMv7-M
        * ARMv7-A - planned

### Command Line Interfaces (CLIs)
* [Alchemist](Alchemist) - .NET Core 3.0 CLI for [Element.NET](Element.NET) ![](https://github.com/ultraleap/Element/workflows/Alchemist.yml/badge.svg)
    * CLI for executing Element via a REPL or compiling to other targets
* [libelement.CLI](libelement.CLI) - C++ CLI for [libelement](libelement)
    * CLI for executing Element via a REPL or compiling to other targets

### Integrations/Tooling
* [Element.Unity](Element.Unity) - Unity using [Element.NET](Element.NET)
    * Node Graph - allows visualization and modification of Element code using an interactive Node Graph
    * Debugger - allows evaluating arbitrary Element code and exploring all of the intermediates in a foldout tree
    * Interactive Workspace - allows evaluating arbitrary Element code and displaying immediate intermediates and visualization (Likely to be merged with Debugger in future)
* [VSCode Linter](element-vscode) - VSCode Extension
    * Syntax highlighting for .ele files
    * Installation guide:
        * Download or clone this repo
        * Extract `element-vscode` to `%UserProfile%/.vscode/extensions`
        * Restart VS Code and enjoy!
