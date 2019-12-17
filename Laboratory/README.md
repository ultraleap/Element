# Laboratory - The Element Host Test Suite
Laboratory is a .NET application depending on NUnit for writing tests for Element Hosts.

## Error Handling in Element

Element has no concept of runtime errors due to static typing and its functional nature.
If compilation succeeds, a function has been compiled which returns valid output for any input (valid doesn't necessarily mean sensible).

That's not to say there isn't a need for error handling - static typing means several classes of errors will be caught at compilation time.
The convention for compilation errors is to return an error message defined within [Messages.toml](../Common/Messages.toml).

### Anatomy of an Error

The convention for defining an error in  is a table defined in the format `ELE####` where `#` is any number `0-9` - this is the message code.
The table then includes a name string, summary string and level string for the given error.
Valid levels are listed at the top of [Messages.toml](../Common/Messages.toml).
```toml
[ELE0000]
name = "ZeroLengthArray"
summary = "Cannot have a zero length array"
level = "Error"
```
When a compiler logs an error it **_must_** a message code as they are used in error case compliance tests.

All other information is considered metadata and the compiler may format it however desired and add it's own context to the message e.g. a stack trace.

## Test Conventions
* Tests should be named after the feature or error case they test, e.g. "Parsing", "CyclicReference".
* Tests are can rely on Element source files from Prelude, the Standard Library or in the Tests directory.
* Tests are run using a command to invoke each compiler under test. See [writing a compliant compiler CLI](#writing-a-compliant-compiler-cli) below.
* Several convention-based tests can be implemented as [compounds](Compounds.md).

### Writing a compliant compiler CLI
Compilers must provide a CLI to be testable in Laboratory and the command for invoking the compiler should be added to CompilerCommands.txt.
Compliant CLIs must implement the following commands:

#### Execute
Compile and executes a function, returning the results of the functions evaluation. 

`execute [-p <packages>...] -f <function> [-a <arguments>...]`

Arguments (order shouldn't matter):
* `-f` is a string identifying the name of the function to compile and execute. Can be namespace qualified.
* (optional) `-a` a list of floats representing the serialized inputs for the function. Not required when evaluating constants.
* (optional) `-p` is a list of strings identifying packages. The only valid package currently is `StandardLibrary`.

Example: `execute -f add -a 1 5` should print `6`

NOTE: An issue with Windows Terminal default encoding causes `∞` to be converted to `8` ([see stackoverflow](https://stackoverflow.com/questions/40907417/why-is-infinity-printed-as-8-in-the-windows-10-console)). To combat this, all `∞` should be converted to the text `Infinity`.