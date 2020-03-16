# Laboratory - The Element Host Test Suite
Laboratory is an NUnit-based .NET test application capable of testing Element Hosts.

## Error Handling in Element

Element has no concept of runtime errors - any compiled function returns valid output for any input (valid doesn't necessarily mean sensible).

That's not to say there isn't a need for error handling - static typing means several classes of errors will be caught at compilation time.
The convention for compilation errors is to return an error message defined within [Messages.toml](../Common/Messages.toml).

### Anatomy of an Error

Errors are defined in [Messages.toml](../Common/Messages.toml) - the convention for error definitions is a table titled `ELE#` where `#` is any number `0-9999`.
The table must include a name and level string for the given error.
Valid error levels are "Verbose", "Information", "Warning" and "Error".
```toml
[ELE7]
name = "IdentifierNotFound"
level = "Error"
```

## Test Conventions
Tests are organized by the level of the compilation stage they test. Each level has individual conventions in addition to the following:
* Tests should be named after the feature or error case they test, e.g. "NestedCallExpression".
* Each test involves calling an Element Host to perform some action. Two kinds of hosts exist:
   * Self Host - laboratory performs the action using Element.NET directly from the test suite.
   * Process Host - laboratory calls into another process using commands to perform actions.
     Details of all process hosts must be defined in [ProcessHostConfigurations.toml](ProcessHostConfigurations.toml).
     These processes must conform to the CLI convention defined below.
* All tests are run for each host instance.

### L0 - Parsing
Parse tests are discovered automatically from the Parsing directory and any subdirectories.
Parse tests must end their filename with either:  
* `-pass` the test expects no errors and will fail on any error
* `-fail` the test expects a parse error and will only succeed if a parse error is generated

The name of a parse test is the relative path from the root Parsing directory including the filename. 
  
### L1 - Validation
Validation tests are responsible for 
Validation tests are discovered automatically from the Validation directory and any subdirectories.
* `-pass` the test expects no errors and will fail on any error
* `-fail-#` where `#` is the message code expected to cause failure. Receiving any other error or no error fails the test. 

The name of a validation test is the relative path from the root Validation directory including the filename. 

### L2 - Semantics
Semantic tests are responsible for ensuring the behaviour of language features.
Semantic tests are written as C# test cases with an accompanying Element file.
The Prelude is not included in semantic tests so that language features are isolated and testable incrementally.
The base class `SemanticsFixture` is used to to capture this convention.

### L3 - Prelude
Prelude tests ensure Prelude intrinsics and functionality is correct and are written as C# test cases.

### L4 - Standard Library
Standard Library tests ensure standard library functionality is correct and are written as C# test cases.

## Command Line Interface (CLI) Convention
All commands are a verb followed by some options with arguments.
Commands have a base set of options. These are:
* `-no-prelude` if specified, the Prelude is not automatically included.
* `-packages <packages>...` includes one or more packages.
* `-source-files <paths>...` includes one or more individual source files.
* `-verbosity` sets the compiler log verbosity. Defaults to "Information".
* `-logjson` log messages as serialized json. Used by the test suite.

### JSON Message Format
A compliant CLI must lost messages in the correct format for the test suite to run as expected.
The JSON message format is as follows:
```json
{
  "MessageCode": 12,
  "MessageLevel": "Error",
  "Context": "SomeContextualInformation",
  "TraceStack": [
    {
      "What": "BadlyWrittenExpression",
      "Line": 10,
      "Column": 25,
      "Source": "MySource.ele"
    }
    ]
}
```
MessageCode, MessageLevel and Context may all be null while TraceStack may be empty.
Bare messages may be passed by only setting Context.

### Commands
A compliant CLIs must implement the following commands:

#### Parse
Parse packages and files provided by base options, returning whether or not parsing was successful.

`parse [-no-validation] [<base options>]`
* `-no-validation` if specified, skips validating parsed files. Errors such as duplicate/invalid identifiers will not be discovered/logged.

##### Example
* `parse` should parse only the Prelude
* `parse -no-prelude -source-files "MySource.ele"` should parse only "MySource.ele"
* `parse -packages "StandardLibrary"` should parse the Prelude and Standard Library

#### Evaluate
Compile and evaluate an expression, returning the result. The result must be a serializable value.

`evaluate -e <expression> [<base options>]`
* `-e <expression>` where expression is a string. Conceptually equivalent to `return = <expression>` written in source.

##### Example
* `evaluate -e "Num.add(4, 2)"` prints `6`
* `evaluate -e "list(4, 5, 6).fold(0, Num.add)"` prints `15`

##### Notes
* An issue with Windows Terminal default encoding causes `∞` to be converted to `8` ([see stackoverflow](https://stackoverflow.com/questions/40907417/why-is-infinity-printed-as-8-in-the-windows-10-console)). To combat this, all returned `∞` should be converted to the text `Infinity`.

#### Typeof
Compile and evaluate an expression, returning the type of the result. Can resolve type of any kind of value, not just serializable values.
`typeof -e <expression> [<base options>]`
* `-e <expression>` where expression is a string. Conceptually equivalent to `return = <expression>` written in source.

##### Example
* `typeof -e "5"` prints "Num"
* `typeof -e "Num.add"` prints "Function"
* `typeof -e "Num"` prints "Type"