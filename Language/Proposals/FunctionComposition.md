Function composition is currently verbose as it requires explicitly repeating function signatures.
```
addAndMul(a, b, c) = a.add(b).mul(c);
```

Composition syntax could reduce verbosity significantly and make code clearer.
```
# given add and mul
add(a, b) = ...;
mul(a, b) = ...;

# applying some composition operator we can curry functions together
addAndMul = add >> mul;
```
The first argument of the first function remains as the first argument of the curried function.

Any non-unary functions also have their arguments hoisted and accumulated to the curried functions parameter list.

Output values of any intermediate functions becomes the first argument in the successive function.
As such, they must be type compatible.
The output of the last composed function becomes the output of the curried function.
```
ternaryFunction(a, b, c) = ...;
binaryFunction(d, e) = ...;

curried = ternaryFunction >> binaryFunction;

# equivalent to
curried(a, b, c, e) = binaryFunction(ternaryFunction(a, b, c), e);
```

Functions are not currently indexable, indexing syntax could be reused but this could lead to confusion.
Additionally this function should work with all function types thus indexing functions would lead to compose introducing either a reserved identifier or overloading.
```
curried = ternary.compose(binaryFunction);
```