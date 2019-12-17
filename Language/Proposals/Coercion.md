Coercion functions could be implemented on types to allow implicitly converting a type to another type.
This prevents the need to repeat manual conversion calls.

Example situation:
```
struct Radians:num
{
    intrinsic sin(a:Radians):num;
    intrinsic cos(a:Radians):num;
    intrinsic tan(a:Radians):num;

    intrinsic asin(a:Radians):num;
    intrinsic acos(a:Radians):num;
    intrinsic atan(a:Radians):num;

    toDegrees(a:Radians):Degrees = a.mul(180.div(num.pi));
}



struct Degrees:num
{
    sin(a:Degrees):num = a.toRadians.sin;
    cos(a:Degrees):num = a.toRadians.cos;
    tan(a:Degrees):num = a.toRadians.tan;

    asin(a:Degrees):num = a.toRadians.asin;
    acos(a:Degrees):num = a.toRadians.acos;
    atan(a:Degrees):num = a.toRadians.atan;

    toRadians(a:Degrees):Radians = a.mul(num.pi.div(180));
}
```

With potential coercion syntax:
```
struct Radians:num
{
    coercion Radians(a:Degrees) = a.mul(num.pi.div(180));
    
    intrinsic sin(a:Radians):num;
    intrinsic cos(a:Radians):num;
    intrinsic tan(a:Radians):num;

    intrinsic asin(a:Radians):num;
    intrinsic acos(a:Radians):num;
    intrinsic atan(a:Radians):num;
}



struct Degrees:num
{
    coercion Degrees(a:Radians) = a.mul(180.div(num.pi));
    
    # No need to repeat definitions as the compiler can coerce Degrees to Radians to call instance functions
}
```

A major drawback is that compilers must traverse the coercion graph to discover what instance functions are valid calls.

Shadowing can also occur when a type contains a function definition and coercion to the same identifier on another type is possible.
This also leads to requiring overload resolution rules.