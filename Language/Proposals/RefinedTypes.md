Subtypes which constrain their fields with particular conditions.

Refined types can be used in place of their base type.

Example refined type implementation:

```
struct Bool:num = base.abs.clamp(0, 1).ceil.to(Bool)
{
    true = Bool(1);
    false = Bool(0);

    if(condition:Bool, ifTrue, ifFalse) = array(ifFalse, ifTrue).at(condition).to(Bool);

    negate(a:Bool):Bool         = a.add(1).rem(2).to(Bool);
    and(a:Bool, b:Bool):Bool    = a.mul(b).to(Bool);
    or(a:Bool, b:Bool):Bool     = a.add(b).sub(a.mul(b)).to(Bool);
    xor(a:Bool, b:Bool):Bool    = a.add(b).rem(2).to(Bool);
    xnor(a:Bool, b:Bool):Bool   = a.add(b).add(1).rem(2).to(Bool);
}



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