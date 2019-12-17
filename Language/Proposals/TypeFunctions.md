Allowing function calls in type expressions such as `foo(a:bar(5))` enables writing type functions which return type instances.

Some usage examples:
```
my_generic(t) {
   struct return(a:t, b:t) {
       foo(a:t) = ...;
    }
}

bar(b : my_generic(num)) = ...;
```

```
baz(t) { return(a:t, b:t):t = some_operation; }

qux = baz(num)(5, 4);
```