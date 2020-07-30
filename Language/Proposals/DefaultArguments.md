Default arguments are required to allow writing functions that behave sensibly by default.

Current situation:
```
MyCircle(radius:Num) = ...;
```
This function's default argument if it were to be compiled would be 0. A circle with 0 radius is nonsensical.

To avoid this situation, programmers must be allowed to specify default arguments:
```
MyCircle(radius:Num = 0.2) = ...;
```

This function can now be compiled to one with a sensible default circle radius.

An additional benefit of this is that programmers can omit the argument when calling if they wish to use the default.

Default arguments should be implemented such that all optional parameters come after all required parameters
to avoid requiring complex logic to deduce which parameters have been supplied. (This could be relaxed in future)

e.g.
```
MyCircle(position:Vector3, radius:Num = 0.2) = ...; # OK
MyCircle(position:Vector3 = Vector3(0, 0, 0.2), radius:Num) = ...; # NOT OK - required parameter after optional one
```