# Element
Element is a minimal functional programming language.
Element code runs using a host which can interpret or compile it to other formats.
One of these formats is the bytecode format, [LMNTIL (Element Intermediate Language)](LMNT/Bytecode.md), for use in native and embedded applications.

### Overhaul
***Content in this repository is currently undergoing large-scale overhaul.
Most content conforms to older undocumented conventions or is broken.***
* Language Reference Manual - Updated
* Prelude - Updated (untested, no compiler has been updated yet)
* Laboratory (Test Suite) - In progress
* Almost everything else - Working using undocumented previous language version or broken

## Element Language
This is a brief summary, see the [Element Reference Manual](Language/ElementReferenceManual.md) for full details.
* Single number data type `num`
* Structured types using `struct`
* First class functions and local functions
* Intrinsics (math intrinsics, collection intrinsics, control flow)
* Genericity via implicit interfaces with `any` type

### Element Libraries
Element includes a library of core functionality called [Prelude](Common/Prelude).

Element also comes with a [Standard Library](Common/StandardLibrary).
New features will generally be implemented as part of the standard library rather than as language features.

## Repository Layout
* [Laboratory - The Element Host Test Suite](Laboratory) - .NET Core 2.1 CLI
    * Test runner using NUnit containing host compliance tests, prelude tests and standard library tests.
    * Can test Element.NET directly or invoke other compilers via CLI convention

### Host Libraries
* [Element.NET](Element.NET) - .NET Standard 2.0
    * Element parser using [Lexico](https://github.com/hamish-milne/Lexico)
    * Element function evaluation via:
        * Direct evaluation (slow)
        * Compilation to CLR Function using [LINQ Expressions](https://docs.microsoft.com/en-us/dotnet/api/system.linq.expressions.expression) (very fast)
    * AoT compilation targets including:
        * [LMNTIL](LMNT/Bytecode.md)
        * C
* [libelement](libelement) - C++
    * Element parser
    * Element function evaluation
    * AoT compilation to:
        * [LMNTIL](LMNT/Bytecode.md)
* [PyElement](PyElement) - Python
    * Element parser using [TatSu](https://github.com/neogeny/TatSu)
    * Element interpreter using Python [eval()](https://docs.python.org/3/library/functions.html)
* [LMNT](LMNT) - C
    * [LMNTIL](LMNT/Bytecode.md) interpreter
    * JIT compiler using [DynASM](https://luajit.org/dynasm.html)
        * x86_64
        * ARMv7-M - in progress
        * ARMv7-A - planned

### Command Line Interfaces (CLIs)
* [Alchemist](Alchemist) - .NET Core 2.1 using [Element.NET](Element.NET)
    * CLI for executing Element via a REPL or compiling to other targets
* [libelement.CLI](libelement.CLI) - C++ using [libelement](libelement)
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
