# LMNT Instructions

These are the possible valid instructions in LMNT and their meaning. Any arguments not mentioned in an instruction's definition or described as reserved are unused by that function and can be assumed to contain an indeterminate value.

**Note that these are currently subject to change at any time.**

## Value types
* **Scalar (S)**: a single value occupying one stack location
* **Vector (V)**: a 4-wide array of values occupying four contiguous stack locations
* **Immediate (I)**: a value encoded in the instruction, either as an integer (II) or a binary value (IB)
* **Reference (R)**: a single value occupying one stack location whose value (cast to integer) is another location, either on the stack or in another table

## Argument types
* **Stack Loc**: an integer representing a location on the function's stack
* **Immediate**: a value encoded into the argument itself
* **Stack Ref**: an integer location on the stack, the value contained in which (cast to integer) is the stack location to act upon
* **Data Ref**: an integer location on the stack, the value contained in which (cast to integer) is the index to act upon in a separately-specified data section
* **Def Pointer**: an integer offset into the archive's defs table, representing a function
* **Code Index**: an integer index into the current function's code entry, effectively pointing to an instruction

## Default assumptions
These assumptions apply to all instructions unless otherwise noted.
* **Safety**: in an archive which has passed validation, instructions cannot cause access violations or otherwise fail.
* **Halting**: the program can be assumed to halt in a predictable amount of time.
* **Dependencies**: instructions are not directly dependent on each other.
* **Aliasing**: input and output stack locations may be aliased. For vector operations, the aliasing must be aligned - i.e. the output location must be the same as the input location(s) or they must not overlap at all.
* **Rounding**: all rounding uses default IEEE754 behaviour.
* **NaNs**: any operation involving one or more NaNs will produce a result of NaN. For memberwise vector operations, other lanes of the operation must not be affected.

# Instruction Reference

## `NOOP`
Takes no action.

Implementations may choose to elide this operation when executing, but **must** respect its position in the function (e.g. when calculating branch targets).


## `RETURN`
Returns from the current function immediately.


## `ASSIGNSS`
Takes a scalar value from the input stack location and writes it to another stack location ("move" in assembly parlance).

| Arg | Direction | Type       | Size   | Meaning                           |
| --: | :-------- | :--------- | :----- | :-------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Stack location to read value from |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write value to  |

```c
    stack[arg3] = stack[arg1]
```


## `ASSIGNVV`
Takes a vector value from a 4-wide set of input stack locations and writes it to another 4-wide set of stack locations ("move" in assembly parlance).

| Arg | Direction | Type       | Size   | Meaning                           |
| --: | :-------- | :--------- | :----- | :-------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First stack location to read value from (read occurs from `arg1+0..arg1+3`) |
| 3   | Output    | Stack Loc  | Vector | First stack location to write value to (write occurs from `arg3+0..arg3+3`) |

```c
    for (i=0..3)
        stack[arg3+i] = stack[arg1+i]
```


## `ASSIGNSV`
Takes a vector value from a single stack location and broadcasts it to a 4-wide set of stack locations.

| Arg | Direction | Type       | Size   | Meaning                           |
| --: | :-------- | :--------- | :----- | :-------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Stack location to read value from |
| 3   | Output    | Stack Loc  | Vector | Stack location to write value to  |

```c
    for (i=0..3)
        stack[arg3+i] = stack[arg1]
```


## `ASSIGNIIS`
Takes an immediate value as a signed integer and writes its value equivalent to a stack location.

| Arg | Direction | Type       | Size       | Meaning                               |
| --: | :-------- | :--------- | :--------  | :------------------------------------ |
| 1   | Input     | Immediate  | LO(SInt32) | Low bits of signed integer immediate  |
| 2   | Input     | Immediate  | HI(SInt32) | High bits of signed integer immediate |
| 3   | Output    | Stack Loc  | Scalar     | Stack location to write value to      |

```c
    stack[arg3] = signed_value_cast(arg1 | (arg2 << sizeof(arg1)))
```


## `ASSIGNIBS`
Takes a binary-encoded value and writes that value to a stack location.

Note: use of this instruction requires that `sizeof(arg1) + sizeof(arg2) == sizeof(lmnt_value)`.

| Arg | Direction | Type       | Size           | Meaning                               |
| --: | :-------- | :--------- | :------------- | :------------------------------------ |
| 1   | Input     | Immediate  | LO(lmnt_value) | Low bits of encoded raw value         |
| 2   | Input     | Immediate  | HI(lmnt_value) | High bits of encoded raw value        |
| 3   | Output    | Stack Loc  | Scalar         | Stack location to write to            |

```c
    stack[arg3] = bit_cast(arg1 | (arg2 << sizeof(arg1)))
```


## `ASSIGNIIV`
Takes an immediate value as a signed integer and broadcasts its value equivalent to a 4-wide set of stack locations.

| Arg | Direction | Type       | Size       | Meaning                               |
| --: | :-------- | :--------- | :--------  | :------------------------------------ |
| 1   | Input     | Immediate  | LO(SInt32) | Low bits of signed integer immediate  |
| 2   | Input     | Immediate  | HI(SInt32) | High bits of signed integer immediate |
| 3   | Output    | Stack Loc  | Vector     | First stack location to write to      |

```c
    for (i=0..3)
        stack[arg3+i] = signed_value_cast(arg1 | (arg2 << sizeof(arg1)))
```


## `ASSIGNIBV`
Takes a binary-encoded value and writes that value to a 4-wide set of stack locations.

Note: use of this instruction requires that `sizeof(arg1) + sizeof(arg2) == sizeof(lmnt_value)`.

| Arg | Direction | Type       | Size           | Meaning                               |
| --: | :-------- | :--------- | :------------- | :------------------------------------ |
| 1   | Input     | Immediate  | LO(lmnt_value) | Low bits of encoded raw value         |
| 2   | Input     | Immediate  | HI(lmnt_value) | High bits of encoded raw value        |
| 3   | Output    | Stack Loc  | Vector         | First stack location to write to      |

```c
    for (i=0..3)
        stack[arg3+i] = bit_cast(arg1 | (arg2 << sizeof(arg1)))
```


## `DLOADIIS`
Reads a constant from the specified data section and index and writes it to a stack location.

This instruction has a limit on the indexes accessible to it, due to the argument size. `DLOADIRS` can be used to read from the end of larger sections.

| Arg | Direction | Type       | Size       | Meaning                                  |
| --: | :-------- | :--------- | :--------  | :--------------------------------------- |
| 1   | Input     | Immediate  | UInt16     | Index of data section to read from       |
| 2   | Input     | Immediate  | UInt16     | Index of data element in section to read |
| 3   | Output    | Stack Loc  | Scalar     | Stack location to write value to         |

```c
    stack[arg3] = data[arg1][arg2]
```


## `DLOADIIV`
Reads a 4-wide vector of constants from the specified data section and index and writes them to four stack locations.

This instruction has a limit on the indexes accessible to it, due to the argument size. `DLOADIRV` can be used to read from the end of larger sections.

| Arg | Direction | Type       | Size       | Meaning                                  |
| --: | :-------- | :--------- | :--------  | :--------------------------------------- |
| 1   | Input     | Immediate  | UInt16     | Index of data section to read from       |
| 2   | Input     | Immediate  | UInt16     | Index of data element in section to read |
| 3   | Output    | Stack Loc  | Vector     | First stack location to write value to   |

```c
    for (i=0..3)
        stack[arg3+i] = data[arg1][arg2+i]
```


## `DLOADIRS`
Reads a constant from the specified data section, using an index sourced from a stack location, and writes it to a stack location.

**Safety**: this instruction allows for dynamically choosing a location to read from in a data section; as a result it is possible for this instruction to cause an access violation error (which will result in the function immediately returning with the `LMNT_ERROR_ACCESS_VIOLATION` code).

| Arg | Direction | Type       | Size       | Meaning                                       |
| --: | :-------- | :--------- | :--------  | :-------------------------------------------- |
| 1   | Input     | Immediate  | UInt16     | Index of data section to read from            |
| 2   | Input     | Data Ref   | Scalar     | Stack location to read data index from        |
| 3   | Output    | Stack Loc  | Scalar     | Stack location to write value to              |

```c
    data_index = (int32)(stack[arg2])
    if (data_index < 0 || data_index + 1 > len(data[arg1])) error(LMNT_ERROR_ACCESS_VIOLATION)
    stack[arg3] = data[arg1][data_index]
```


## `DLOADIRV`
Reads a 4-wide vector of constants from the specified data section and index and writes them to four stack locations.

**Safety**: this instruction allows for dynamically choosing a location to read from in a data section; as a result it is possible for this instruction to cause an access violation error (which will result in the function immediately returning with the `LMNT_ERROR_ACCESS_VIOLATION` code).

| Arg | Direction | Type       | Size       | Meaning                                       |
| --: | :-------- | :--------- | :--------  | :-------------------------------------------- |
| 1   | Input     | Immediate  | UInt16     | Index of data section to read from            |
| 2   | Input     | Data Ref   | UInt16     | Stack location to read first data index from  |
| 3   | Output    | Stack Loc  | Vector     | First stack location to write value to        |

```c
    data_index = (int32)(stack[arg2])
    if (data_index < 0 || data_index + 4 > len(data[arg1])) error(LMNT_ERROR_ACCESS_VIOLATION)
    for (i=0..3)
        stack[arg3+i] = data[arg1][data_index+i]
```


## `DSECLEN`
Writes the length, in entries, of the specified data section to a stack location.

| Arg | Direction | Type       | Size       | Meaning                                 |
| --: | :-------- | :--------- | :--------  | :-------------------------------------- |
| 1   | Input     | Immediate  | UInt16     | Index of data section to query          |
| 3   | Output    | Stack Loc  | Scalar     | Stack location to write value to        |

```c
    stack[arg3] = (lmnt_value)(len(data[arg1]))
```


## `ADDSS`
Adds two scalar values and writes the result to a third scalar.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of first value to operate on  |
| 2   | Input     | Stack Loc  | Scalar | Location of second value to operate on |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = stack[arg1] + stack[arg2]
```

## `ADDVV`
Memberwise adds two vector values and writes the result to a third vector.

| Arg | Direction | Type       | Size   | Meaning                                       |
| --: | :-------- | :--------- | :----- | :-------------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of first values to operate on  |
| 2   | Input     | Stack Loc  | Vector | First location of second values to operate on |
| 3   | Output    | Stack Loc  | Vector | First stack location to write results to      |

```c
    for (i=0..3)
    stack[arg3+i] = stack[arg1+i] + stack[arg2+i]
```


## `SUBSS`
Subtracts one scalar value from another and writes the result to a third scalar.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of first value to operate on  |
| 2   | Input     | Stack Loc  | Scalar | Location of second value to operate on |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = stack[arg1] - stack[arg2]
```


## `SUBVV`
Memberwise subtracts two vector values and writes the result to a third vector.

| Arg | Direction | Type       | Size   | Meaning                                       |
| --: | :-------- | :--------- | :----- | :-------------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of first values to operate on  |
| 2   | Input     | Stack Loc  | Vector | First location of second values to operate on |
| 3   | Output    | Stack Loc  | Vector | First stack location to write results to      |

```c
    for (i=0..3)
        stack[arg3+i] = stack[arg1+i] - stack[arg2+i]
```


## `MULSS`
Multiplies one scalar value with another and writes the result to a third scalar.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of first value to operate on  |
| 2   | Input     | Stack Loc  | Scalar | Location of second value to operate on |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = stack[arg1] * stack[arg2]
```


## `MULVV`
Memberwise multiplies two vector values and writes the result to a third vector.

| Arg | Direction | Type       | Size   | Meaning                                       |
| --: | :-------- | :--------- | :----- | :-------------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of first values to operate on  |
| 2   | Input     | Stack Loc  | Vector | First location of second values to operate on |
| 3   | Output    | Stack Loc  | Vector | First stack location to write results to      |

```c
    for (i=0..3)
        stack[arg3+i] = stack[arg1+i] * stack[arg2+i]
```


## `DIVSS`
Divides one scalar value by another and writes the result to a third scalar.

Division by zero results in the appropriate infinity for floating-point value types, and is undefined for integer value types.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of first value to operate on  |
| 2   | Input     | Stack Loc  | Scalar | Location of second value to operate on |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = stack[arg1] / stack[arg2]
```


## `DIVVV`
Memberwise divides two vector values and writes the result to a third vector.

Division by zero results in the appropriate infinity for floating-point value types, and is undefined for integer value types. Occurrences of this must have no impact on the other lanes of the operation.

| Arg | Direction | Type       | Size   | Meaning                                       |
| --: | :-------- | :--------- | :----- | :-------------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of first values to operate on  |
| 2   | Input     | Stack Loc  | Vector | First location of second values to operate on |
| 3   | Output    | Stack Loc  | Vector | First stack location to write results to      |

```c
    for (i=0..3)
        stack[arg3+i] = stack[arg1+i] / stack[arg2+i]
```


## `MODSS`
Performs a modulo operation with two scalar values and writes the result to a third scalar.

If the denominator value is `0`, the result will be `NaN`.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of first value to operate on  |
| 2   | Input     | Stack Loc  | Scalar | Location of second value to operate on |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = stack[arg1] % stack[arg2]
```


## `MODVV`
Memberwise performs a modulo operation on two vector values and writes the result to a third vector.

If a denominator value is `0`, the result will be `NaN`. Occurrences of this must have no impact on the other lanes of the operation.

| Arg | Direction | Type       | Size   | Meaning                                       |
| --: | :-------- | :--------- | :----- | :-------------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of first values to operate on  |
| 2   | Input     | Stack Loc  | Vector | First location of second values to operate on |
| 3   | Output    | Stack Loc  | Vector | First stack location to write results to      |

```c
    for (i=0..3)
        stack[arg3+i] = stack[arg1+i] % stack[arg2+i]
```


## `SIN`
Performs the sine trigonometric operation on the input value and writes the result to another location.

| Arg | Direction | Type       | Size   | Meaning                                     |
| --: | :-------- | :--------- | :----- | :------------------------------------------ |
| 1   | Input     | Stack Loc  | Scalar | Location of the input to the sine operation |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write the result to       |

```c
    stack[arg3] = sin(stack[arg1])
```

## `COS`
Performs the cosine trigonometric operation on the input value and writes the result to another location.

| Arg | Direction | Type       | Size   | Meaning                                       |
| --: | :-------- | :--------- | :----- | :-------------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of the input to the cosine operation |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write the result to         |

```c
    stack[arg3] = cos(stack[arg1])
```


## `TAN`
Performs the tangent trigonometric operation on the input value and writes the result to another location.

| Arg | Direction | Type       | Size   | Meaning                                        |
| --: | :-------- | :--------- | :----- | :--------------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of the input to the tangent operation |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write the result to          |

```c
    stack[arg3] = tan(stack[arg1])
```


## `ASIN`
Performs the arcsine trigonometric operation on the input value and writes the result to another location.

| Arg | Direction | Type       | Size   | Meaning                                        |
| --: | :-------- | :--------- | :----- | :--------------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of the input to the arcsine operation |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write the result to          |

```c
    stack[arg3] = asin(stack[arg1])
```


## `ACOS`
Performs the arccosine trigonometric operation on the input value and writes the result to another location.

| Arg | Direction | Type       | Size   | Meaning                                          |
| --: | :-------- | :--------- | :----- | :----------------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of the input to the arccosine operation |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write the result to            |

```c
    stack[arg3] = acos(stack[arg1])
```


## `ATAN`
Performs the arctangent trigonometric operation on the input value and writes the result to another location.

| Arg | Direction | Type       | Size   | Meaning                                           |
| --: | :-------- | :--------- | :----- | :------------------------------------------------ |
| 1   | Input     | Stack Loc  | Scalar | Location of the input to the arctangent operation |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write the result to             |

```c
    stack[arg3] = atan(stack[arg1])
```


## `ATAN2`
Performs the two-argument arctangent trigonometric operation on the input value and writes the result to another location.

| Arg | Direction | Type       | Size   | Meaning                                               |
| --: | :-------- | :--------- | :----- | :---------------------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of the "y" input to the arctangent operation |
| 1   | Input     | Stack Loc  | Scalar | Location of the "x" input to the arctangent operation |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write the result to                 |

```c
    stack[arg3] = atan2(stack[arg1], stack[arg2])
```


## `SINCOS`
Performs both sine and cosine calculations as a single operation. If both are required, some implementations may be able to use optimised routines to perform the operations faster.

| Arg | Direction | Type       | Size   | Meaning                                             |
| --: | :-------- | :--------- | :----- | :-------------------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of the input to the sine/cosine operations |
| 2   | Output    | Stack Loc  | Scalar | Stack location to write the sine result to          |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write the cosine result to        |

```c
    stack[arg2], stack[arg3] = sincos(stack[arg1])
```


## `POWSS`
Takes a value to the power of another value, and writes the result.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of base value                 |
| 2   | Input     | Stack Loc  | Scalar | Location of exponent value             |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = pow(stack[arg1], stack[arg2])
```


## `POWVV`
Memberwise performs the power (exponent) operation on a vector of values by another vector, and writes the result to another vector.

| Arg | Direction | Type       | Size   | Meaning                                      |
| --: | :-------- | :--------- | :----- | :------------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of base values                |
| 2   | Input     | Stack Loc  | Vector | First location of exponent values            |
| 3   | Output    | Stack Loc  | Vector | First stack location to write results to     |

```c
    for (i=0..3)
        stack[arg3+i] = pow(stack[arg1+i], stack[arg2+i])
```


## `POWVS`
Memberwise performs the power (exponent) operation on a vector of values by a scalar, and writes the result to another vector.

| Arg | Direction | Type       | Size   | Meaning                                      |
| --: | :-------- | :--------- | :----- | :------------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of base values                |
| 2   | Input     | Stack Loc  | Scalar | Location of exponent value                   |
| 3   | Output    | Stack Loc  | Vector | First stack location to write results to     |

```c
    for (i=0..3)
        stack[arg3+i] = pow(stack[arg1+i], stack[arg2])
```


## `SQRTS`
Performs a square root operation on a value, and stores it in another location.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of input value                |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = sqrt(stack[arg1])
```

## `SQRTV`
Performs a square root operation on each value in a 4-wide vector, and stores the results in another location.

| Arg | Direction | Type       | Size   | Meaning                                 |
| --: | :-------- | :--------- | :----- | :-------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of input values          |
| 3   | Output    | Stack Loc  | Vector | First stack location to write result to |

```c
    for (i=0..3)
        stack[arg3+i] = sqrt(stack[arg1+i])
```


## `LOG`
Takes the logarithm of a value, also specifying the base, and writes the result.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of the logarithm input        |
| 2   | Input     | Stack Loc  | Scalar | Location of the logarithm base         |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = log(stack[arg1], stack[arg2])
```


## `LN`
Takes the natural logarithm of an input value, and writes the result.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of the logarithm input        |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = ln(stack[arg1])
```


## `LOG2`
Takes the base-2 logarithm of an input value, and writes the result.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of the logarithm input        |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = log2(stack[arg1])
```


## `LOG10`
Takes the base-10 logarithm of an input value, and writes the result.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of the logarithm input        |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = log10(stack[arg1])
```


## `ABSS`
Calculates the absolute value of a value and stores the result in another location.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of input value                |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = abs(stack[arg1])
```


## `ABSV`
Calculates the absolute values of each of a 4-wide vector and stores the result in another vector.

| Arg | Direction | Type       | Size   | Meaning                                 |
| --: | :-------- | :--------- | :----- | :-------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of input values          |
| 3   | Output    | Stack Loc  | Vector | First stack location to write result to |

```c
    for (i=0..3)
        stack[arg3+i] = abs(stack[arg1+i])
```


## `SUMV`
Calculates the sum of a 4-wide vector of values, and stores the result as a scalar in another location.

* **NaNs**: if any of the input values are NaN, the result will be NaN.

| Arg | Direction | Type       | Size   | Meaning                                 |
| --: | :-------- | :--------- | :----- | :-------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of input values          |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write the sum to      |

```c
    stack[arg3] = stack[arg1+0] + stack[arg1+1] + stack[arg1+2] + stack[arg1+3]
```


## `MINSS`
Given two values, stores the lower (more negative) of the two values in another location.

**NaNs**: the behaviour of this function if either operation is a NaN is undefined.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of first input                |
| 2   | Input     | Stack Loc  | Scalar | Location of second input               |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = min(stack[arg1], stack[arg2])
```

## `MINVV`
Given two vectors, stores the lower (more negative) of each lane's values in another location.

**NaNs**: the behaviour of a lane of this function is undefined if either input is NaN; other lanes must not be affected if this occurs.

| Arg | Direction | Type       | Size   | Meaning                                      |
| --: | :-------- | :--------- | :----- | :------------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of first input                |
| 2   | Input     | Stack Loc  | Vector | First location of second input               |
| 3   | Output    | Stack Loc  | Vector | First stack location to write results to     |

```c
    for (i=0..3)
        stack[arg3+i] = min(stack[arg1+i], stack[arg2+i])
```


## `MAXSS`
Given two values, stores the higher (more positive) of the two values in another location.

**NaNs**: the behaviour of this function if either operation is a NaN is undefined.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of first input                |
| 2   | Input     | Stack Loc  | Scalar | Location of second input               |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = max(stack[arg1], stack[arg2])
```

## `MAXVV`
Given two vectors, stores the higher (more positive) of each lane's values in another location.

**NaNs**: the behaviour of a lane of this function is undefined if either input is NaN; other lanes must not be affected if this occurs.

| Arg | Direction | Type       | Size   | Meaning                                      |
| --: | :-------- | :--------- | :----- | :------------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of first input                |
| 2   | Input     | Stack Loc  | Vector | First location of second input               |
| 3   | Output    | Stack Loc  | Vector | First stack location to write results to     |

```c
    for (i=0..3)
        stack[arg3+i] = max(stack[arg1+i], stack[arg2+i])
```


## `MINVS`
Given a vector and a scalar, stores the lower (more negative) of each lane of the vector and the scalar.

**NaNs**: the behaviour of a lane of this function is undefined if either input in that lane is NaN; other lanes must not be affected if this occurs.

| Arg | Direction | Type       | Size   | Meaning                                      |
| --: | :-------- | :--------- | :----- | :------------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of first input                |
| 2   | Input     | Stack Loc  | Scalar | Location of second input                     |
| 3   | Output    | Stack Loc  | Vector | First stack location to write results to     |

```c
    for (i=0..3)
        stack[arg3+i] = min(stack[arg1+i], stack[arg2])
```


## `MAXVS`
Given a vector and a scalar, stores the higher (more positive) of each lane of the vector and the scalar.

**NaNs**: the behaviour of a lane of this function is undefined if either input in that lane is NaN; other lanes must not be affected if this occurs.

| Arg | Direction | Type       | Size   | Meaning                                      |
| --: | :-------- | :--------- | :----- | :------------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of first input                |
| 2   | Input     | Stack Loc  | Scalar | Location of second input                     |
| 3   | Output    | Stack Loc  | Vector | First stack location to write results to     |

```c
    for (i=0..3)
        stack[arg3+i] = max(stack[arg1+i], stack[arg2])
```


## `FLOORS`
Takes the nearest integer below the given value, and stores it in another location.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of input value                |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = floor(stack[arg1])
```


## `FLOORV`
Takes the nearest integer below the given value for each lane of a 4-wide vector, and stores the results in another location.

| Arg | Direction | Type       | Size   | Meaning                                 |
| --: | :-------- | :--------- | :----- | :-------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of input values          |
| 3   | Output    | Stack Loc  | Vector | First stack location to write result to |

```c
    for (i=0..3)
        stack[arg3+i] = floor(stack[arg1+i])
```


## `ROUNDS`
Takes the closest integer to the given value, and stores it in another location.

**Rounding**: in the event of a tie (e.g. exactly `X.5`) the result must be rounded to even.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of input value                |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = round(stack[arg1])
```

Note: the C functions [`round`](https://en.cppreference.com/w/c/numeric/math/round) et al do **not** round to even on a tie; [`rint`](https://en.cppreference.com/w/c/numeric/math/rint) and [`nearbyint`](https://en.cppreference.com/w/c/numeric/math/nearbyint) do when using the default rounding mode.


## `ROUNDV`
Takes the closest integer to the given value for each lane of a 4-wide vector, and stores the results in another location.

**Rounding**: in the event of a tie (e.g. exactly `X.5`) the result must be rounded to even.

| Arg | Direction | Type       | Size   | Meaning                                 |
| --: | :-------- | :--------- | :----- | :-------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of input values          |
| 3   | Output    | Stack Loc  | Vector | First stack location to write result to |

```c
    for (i=0..3)
        stack[arg3+i] = round(stack[arg1+i])
```

Note: the C functions [`round`](https://en.cppreference.com/w/c/numeric/math/round) et al do **not** round to even on a tie; [`rint`](https://en.cppreference.com/w/c/numeric/math/rint) and [`nearbyint`](https://en.cppreference.com/w/c/numeric/math/nearbyint) do when using the default rounding mode.

## `CEILS`
Takes the nearest integer above the given value, and stores it in another location.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of input value                |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = ceil(stack[arg1])
```


## `CEILV`
Takes the nearest integer above the given value for each lane of a 4-wide vector, and stores the results in another location.

| Arg | Direction | Type       | Size   | Meaning                                 |
| --: | :-------- | :--------- | :----- | :-------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of input values          |
| 3   | Output    | Stack Loc  | Vector | First stack location to write result to |

```c
    for (i=0..3)
        stack[arg3+i] = ceil(stack[arg1+i])
```

## `TRUNCS`
Takes the next integer towards zero to the given value, and stores it in another location.

| Arg | Direction | Type       | Size   | Meaning                                |
| --: | :-------- | :--------- | :----- | :------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of input value                |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to      |

```c
    stack[arg3] = trunc(stack[arg1])
```


## `TRUNCV`
Takes the next integer towards zero to the given value for each lane of a 4-wide vector, and stores the results in another location.

| Arg | Direction | Type       | Size   | Meaning                                 |
| --: | :-------- | :--------- | :----- | :-------------------------------------- |
| 1   | Input     | Stack Loc  | Vector | First location of input values          |
| 3   | Output    | Stack Loc  | Vector | First stack location to write result to |

```c
    for (i=0..3)
        stack[arg3+i] = trunc(stack[arg1+i])
```


## `INDEXRIS`
Reads a value from the stack, adds a constant to it, and then reads from *that* stack index, writing it to another stack location.

**Safety**: this instruction allows for dynamically choosing a stack location to read from; as a result it is possible for this instruction to cause an access violation error (which will result in the function immediately returning with the `LMNT_ERROR_ACCESS_VIOLATION` code).

| Arg | Direction | Type       | Size   | Meaning                                                  |
| --: | :-------- | :--------- | :----- | :------------------------------------------------------- |
| 1   | Input     | Stack Ref  | Scalar | Stack location of stack value representing index to read |
| 2   | Input     | Immediate  | UInt16 | Constant to add to retrieved index                       |
| 3   | Output    | Stack Loc  | Scalar | Stack location to write result to                        |

```c
    read_index = (int32)(stack[arg1]) + arg2
    if (read_index < 0 || read_index >= len(stack)) error(LMNT_ERROR_ACCESS_VIOLATION)
    stack[arg3] = stack[read_index]
```


## `INDEXRIR`
Reads a value from the stack, adds a constant to it, and then reads from *that* stack index; then reads the output index from the stack, before writing it to that retrieved location.

**Safety**: this instruction allows for dynamically choosing a stack location to read from and write to; as a result it is possible for this instruction to cause an access violation error (which will result in the function immediately returning with the `LMNT_ERROR_ACCESS_VIOLATION` code).

| Arg | Direction | Type       | Size   | Meaning                                                          |
| --: | :-------- | :--------- | :----- | :--------------------------------------------------------------- |
| 1   | Input     | Stack Ref  | Scalar | Stack location of stack value representing index to read         |
| 2   | Input     | Immediate  | UInt16 | Constant to add to retrieved index                               |
| 3   | Input     | Stack Ref  | Scalar | Stack location of stack value representing index to store result |

```c
    read_index = (int32)(stack[arg1]) + arg2
    write_index = (int32)(stack[arg3])
    if (read_index < 0 || read_index >= len(stack))   error(LMNT_ERROR_ACCESS_VIOLATION)
    if (write_index < 0 || write_index >= len(stack)) error(LMNT_ERROR_ACCESS_VIOLATION)
    stack[write_index] = stack[read_index]
```


## `BRANCH`
Unconditionally branch to the given instruction; the branch target is specified by its absolute index within the current function.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Reserved   |            |                                           |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    goto instructions[target]
```

## `BRANCHZ`
Branch to the given instruction if the test argument is zero; the branch target is specified by its absolute index within the current function. Negative zero (`-0.0`) is counted as zero.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

**NaNs**: if the test argument is NaN, the branch is not taken.

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar     | Location of value to test                 |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    if (stack[arg1] == 0.0)
        goto instructions[target]
```


## `BRANCHNZ`
Branch to the given instruction if the test argument is non-zero; the branch target is specified by its absolute index within the current function. Negative zero (`-0.0`) is counted as zero.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

**NaNs**: if the test argument is NaN, the branch is not taken.

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar     | Location of value to test                 |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    if (stack[arg1] != 0.0)
        goto instructions[target]
```

## `BRANCHPOS`
Branch to the given instruction if the test argument is positive; the branch target is specified by its absolute index within the current function.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

**NaNs**: the branch will be taken if the test argument is a positive NaN.

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar     | Location of value to test                 |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    if (signbit(stack[arg1]) == 0)
        goto instructions[target]
```


## `BRANCHNEG`
Branch to the given instruction if the test argument is negative; the branch target is specified by its absolute index within the current function.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

**NaNs**: the branch will be taken if the test argument is a negative NaN.

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar     | Location of value to test                 |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    if (signbit(stack[arg1]) == 1)
        goto instructions[target]
```


## `BRANCHUN`
Branch to the given instruction if the test argument is `NaN`; the branch target is specified by its absolute index within the current function.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar     | Location of value to test                 |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    if (isnan(stack[arg1]))
        goto instructions[target]
```


## `CMP`
Compares the two specified inputs, and stores the result in an internal set of flags.

The flags stored are:
* **`EQ` (equal)**: the first argument compares equal to the second argument
* **`LT` (less-than)**: the first argument compares less than the second argument
* **`GT` (greater-than)**: the first argument compares greater than the second argument
* **`UN` (unordered)**: the comparison is unordered (i.e. at least one argument was `NaN`)

**NaNs**: if one or both arguments are `NaN`, then `UN` will be set and no other flags will.

| Arg | Direction | Type       | Size   | Meaning                                 |
| --: | :-------- | :--------- | :----- | :-------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of the first input to compare  |
| 2   | Input     | Stack Loc  | Scalar | Location of the second input to compare |

```c
    flags.EQ = (stack[arg1] == stack[arg2])
    flags.LT = (stack[arg1] <  stack[arg2])
    flags.GT = (stack[arg1] >  stack[arg2])
    flags.UN = (isnan(stack[arg1]) || isnan(stack[arg2]))
```


## `CMPZ`
Compares the specified input against zero, and stores the result in an internal set of flags.

The flags stored are:
* **`EQ` (equal)**: the first argument compares equal to the second argument
* **`LT` (less-than)**: the first argument compares less than the second argument
* **`GT` (greater-than)**: the first argument compares greater than the second argument
* **`UN` (unordered)**: the comparison is unordered (i.e. at least one argument was `NaN`)

**NaNs**: if one or both arguments are `NaN`, then `UN` will be set and no other flags will.

| Arg | Direction | Type       | Size   | Meaning                                 |
| --: | :-------- | :--------- | :----- | :-------------------------------------- |
| 1   | Input     | Stack Loc  | Scalar | Location of the first input to compare  |

```c
    flags.EQ = (stack[arg1] == 0.0)
    flags.LT = (stack[arg1] <  0.0)
    flags.GT = (stack[arg1] >  0.0)
    flags.UN = (isnan(stack[arg1]))
```


## `BRANCHCEQ`
Branch to the target instruction if the last `CMP` or `CMPZ` instruction produced an equal-to result; the branch target is specified by its absolute index within the current function.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Reserved   |            |                                           |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    if (flags.EQ)
        goto instructions[target]
```


## `BRANCHCNE`
Branch to the target instruction if the last `CMP` or `CMPZ` instruction produced a not-equal result; the branch target is specified by its absolute index within the current function.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

**NaNs**: if one or both of the prior comparison's arguments are `NaN`, the branch will be taken (since `EQ` will not be set)

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Reserved   |            |                                           |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    if (!flags.EQ)
        goto instructions[target]
```


## `BRANCHCLT`
Branch to the target instruction if the last `CMP` or `CMPZ` instruction produced a less-than result; the branch target is specified by its absolute index within the current function.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Reserved   |            |                                           |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    if (flags.LT)
        goto instructions[target]
```


## `BRANCHCLE`
Branch to the target instruction if the last `CMP` or `CMPZ` instruction produced a less-than or equal-to result; the branch target is specified by its absolute index within the current function.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Reserved   |            |                                           |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    if (flags.LT || flags.EQ)
        goto instructions[target]
```


## `BRANCHCGT`
Branch to the target instruction if the last `CMP` or `CMPZ` instruction produced a greater-than result; the branch target is specified by its absolute index within the current function.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Reserved   |            |                                           |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    if (flags.GT)
        goto instructions[target]
```


## `BRANCHCGE`
Branch to the target instruction if the last `CMP` or `CMPZ` instruction produced a greater-than or equal-to result; the branch target is specified by its absolute index within the current function.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Reserved   |            |                                           |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    if (flags.GT || flags.EQ)
        goto instructions[target]
```


## `BRANCHCUN`
Branch to the target instruction if the last `CMP` or `CMPZ` instruction produced an unordered result; the branch target is specified by its absolute index within the current function.

Note that the branch target is permitted to be equal to the instruction count (i.e. "one past the end"); this is effectively equivalent to a `RETURN`.

**Halting**: if a branch's target is not later than the branch instruction in the function, halting cannot be guaranteed and execution time cannot be easily predicted.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type       | Size       | Meaning                                   |
| --: | :-------- | :--------- | :--------- | :---------------------------------------- |
| 1   | Input     | Reserved   |            |                                           |
| 2   | Input     | Code Index | LO(UInt32) | Low half of the target instruction index  |
| 3   | Input     | Code Index | HI(UInt32) | High half of the target instruction index |

```c
    target = (arg2 | (arg3 << sizeof(arg2)))
    if (target > len(instructions)) error(LMNT_ERROR_ACCESS_VIOLATION)
    if (flags.UN)
        goto instructions[target]
```


## `ASSIGNCEQ`
Assign to the target stack location one of two values, depending on whether the last `CMP` or `CMPZ` instruction produced an equal result.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type      | Size   | Meaning                                                |
| --: | :-------- | :-------- | :----- | :----------------------------------------------------- |
| 1   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is true      |
| 2   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is false     |
| 3   | Output    | Stack Loc | Scalar | Stack location to assign the result to                 |

```c
    if (flags.EQ)
        stack[arg3] = stack[arg1]
    else
        stack[arg3] = stack[arg2]
```


## `ASSIGNCNE`
Assign to the target stack location one of two values, depending on whether the last `CMP` or `CMPZ` instruction produced a not-equal result.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type      | Size   | Meaning                                                |
| --: | :-------- | :-------- | :----- | :----------------------------------------------------- |
| 1   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is true      |
| 2   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is false     |
| 3   | Output    | Stack Loc | Scalar | Stack location to assign the result to                 |

```c
    if (!flags.EQ)
        stack[arg3] = stack[arg1]
    else
        stack[arg3] = stack[arg2]
```


## `ASSIGNCLT`
Assign to the target stack location one of two values, depending on whether the last `CMP` or `CMPZ` instruction produced a less-than result.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type      | Size   | Meaning                                                |
| --: | :-------- | :-------- | :----- | :----------------------------------------------------- |
| 1   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is true      |
| 2   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is false     |
| 3   | Output    | Stack Loc | Scalar | Stack location to assign the result to                 |

```c
    if (flags.LT)
        stack[arg3] = stack[arg1]
    else
        stack[arg3] = stack[arg2]
```


## `ASSIGNCLE`
Assign to the target stack location one of two values, depending on whether the last `CMP` or `CMPZ` instruction produced a less-than or equal result.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type      | Size   | Meaning                                                |
| --: | :-------- | :-------- | :----- | :----------------------------------------------------- |
| 1   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is true      |
| 2   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is false     |
| 3   | Output    | Stack Loc | Scalar | Stack location to assign the result to                 |

```c
    if (flags.LT || flags.EQ)
        stack[arg3] = stack[arg1]
    else
        stack[arg3] = stack[arg2]
```


## `ASSIGNCGT`
Assign to the target stack location one of two values, depending on whether the last `CMP` or `CMPZ` instruction produced a greater-than result.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type      | Size   | Meaning                                                |
| --: | :-------- | :-------- | :----- | :----------------------------------------------------- |
| 1   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is true      |
| 2   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is false     |
| 3   | Input     | Stack Loc | Scalar | Stack location to assign the result to                 |

```c
    if (flags.GT)
        stack[arg3] = stack[arg1]
    else
        stack[arg3] = stack[arg2]
```


## `ASSIGNCGE`
Assign to the target stack location one of two values, depending on whether the last `CMP` or `CMPZ` instruction produced a greater-than or equal result.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type      | Size   | Meaning                                                |
| --: | :-------- | :-------- | :----- | :----------------------------------------------------- |
| 1   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is true      |
| 2   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is false     |
| 3   | Input     | Stack Loc | Scalar | Stack location to assign the result to                 |

```c
    if (flags.GT || flags.EQ)
        stack[arg3] = stack[arg1]
    else
        stack[arg3] = stack[arg2]
```


## `ASSIGNCUN`
Assign to the target stack location one of two values, depending on whether the last `CMP` or `CMPZ` instruction produced an unordered result.

**Dependencies**: this instruction is only guaranteed to perform as expected if it is immediately preceded by a comparison function (`CMP` or `CMPZ`), or if the only intervening instructions are other `BRANCHCxx` or `ASSIGNCxx` instructions.

| Arg | Direction | Type      | Size   | Meaning                                                |
| --: | :-------- | :-------- | :----- | :----------------------------------------------------- |
| 1   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is true      |
| 2   | Input     | Stack Loc | Scalar | Location of value to assign if comparison is false     |
| 3   | Input     | Stack Loc | Scalar | Stack location to assign the result to                 |

```c
    if (flags.UN)
        stack[arg3] = stack[arg1]
    else
        stack[arg3] = stack[arg2]
```


## `EXTCALL`
Calls out to a runtime-defined function, specified via an address into the archive's `defs` table. This allows specific runtimes to add functionality not normally available using LMNT, at the cost of non-portability.

Any archive containing an external def with a name and signature which does not match those known to the runtime will also fail to validate. This mechanism can also be disabled entirely at compile-time; in this instance, any archive containing any defs marked as external will fail to validate.

Summarily: this functionality is discouraged in most cases, but can allow for a wider range of functionality without otherwise compromising the guarantees of the system.

**Halting**: since this instruction calls out to platform-controlled code, there is no guarantee that said code will halt and thus the same is true of this instruction.

| Arg | Direction | Type        | Size       | Meaning                                               |
| --: | :-------- | :---------- | :--------- | :---------------------------------------------------- |
| 1   | Input     | Def Pointer | LO(UInt32) | Low half of the extcall's def address                 |
| 2   | Input     | Def Pointer | HI(UInt32) | High half of the extcall's def address                |
| 3   | Output    | Stack Loc   | Scalar     | First stack location of the arguments + return values |

```c
    def_address = (arg1 | (arg2 << sizeof(arg1)))
    def = find_extern(def_address)
    if (!def.function) error(LMNT_ERROR_NOT_FOUND)
    args = stack + arg3
    rvals = stack + arg3 + def.args_count
    def.function(args, def.args_count, rvals, def.rvals_count)
```

