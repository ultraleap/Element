Subtypes which constrain their fields with particular conditions.

Refined types can be used in place of their base type.

Example refined type implementation:

```
struct Bool:Num = base.abs.clamp(0, 1).ceil.to(Bool)
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



struct Radians:Num
{
    intrinsic sin(a:Radians):Num;
    intrinsic cos(a:Radians):Num;
    intrinsic tan(a:Radians):Num;

    intrinsic asin(a:Radians):Num;
    intrinsic acos(a:Radians):Num;
    intrinsic atan(a:Radians):Num;

    toDegrees(a:Radians):Degrees = a.mul(180.div(Num.pi));
}



struct Degrees:Num
{
    sin(a:Degrees):Num = a.toRadians.sin;
    cos(a:Degrees):Num = a.toRadians.cos;
    tan(a:Degrees):Num = a.toRadians.tan;

    asin(a:Degrees):Num = a.toRadians.asin;
    acos(a:Degrees):Num = a.toRadians.acos;
    atan(a:Degrees):Num = a.toRadians.atan;

    toRadians(a:Degrees):Radians = a.mul(Num.pi.div(180));
}
```