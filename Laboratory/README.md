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
[ELE13]
name = "ZeroLengthArray"
level = "Error"
```
When a compiler logs an error it **_must_** a message code as they are used in error case compliance tests.

All other information is considered metadata and the compiler may format it however desired and add it's own context to the message e.g. a stack trace.

## Test Conventions
* Tests are organized by the level of the compilation stage they test. Each level has individual conventions in addition to those mentioned here.
  * [L0 - Syntax Parsing](L0-Syntax-Parsing)
  * [L1 - Language Semantics](L1-Language-Semantics)
  * [L2 - Prelude](L2-Prelude)
  * [L3 - Standard Library](L3-Standard-Library)
* Tests should be named after the feature or error case they test, e.g. "CyclicReference".
* Each test involves calling an Element Host to perform some action. Two kinds of hosts exist:
   * Self Host - laboratory performs the action using Element.NET.
   * Process Host - laboratory calls into another process (usually a CLI) to perform the action.
   This serves 2 purposes, testing the CLI application itself and allowing Laboratory to easily interop with hosts implemented in different languages.
   See [writing a compliant compiler CLI](#writing-a-compliant-compiler-cli) below.
* All tests are run for each host instance.
* Tests are can rely on Element source files from Prelude, the Standard Library or in the Tests directory.
These are not included by default in most levels and must be included by passing a non-default HostContext to the Host.

### Writing a compliant host CLI
Hosts must provide a CLI to be testable in Laboratory and the details for the CLI should be added to [ProcessHostConfigurations.toml](ProcessHostConfigurations.toml).
A compliant CLIs must implement the following commands:

#### Execute
Compile and executes a function, returning the results of the functions evaluation. 

`execute [-p <packages>...] -f <function> [-a <arguments>...]`

Arguments (order shouldn't matter):
* `-f` is a string identifying the name of the function to compile and execute. Can be namespace qualified.
* (optional) `-a` a list of floats representing the serialized inputs for the function. Not required when evaluating constants.
* (optional) `-p` is a list of strings identifying packages. The only valid package currently is `StandardLibrary`.

Example: `execute -f add -a 1 5` should print `6`

NOTE: An issue with Windows Terminal default encoding causes `∞` to be converted to `8` ([see stackoverflow](https://stackoverflow.com/questions/40907417/why-is-infinity-printed-as-8-in-the-windows-10-console)). To combat this, all `∞` should be converted to the text `Infinity`.

#### Parse
Parse a file, returning whether or not parsing was successful.

`parse [] -f <files>...`