using System;

namespace Element.AST
{
    public class NullaryIntrinsics : IntrinsicFunction
    {
        private readonly Value _value;

        public enum Value
        {
            NaN,
            PositiveInfinity,
            NegativeInfinity,
        }

        public NullaryIntrinsics(Value value)
            : base($"Num.{value.ToString()}", Array.Empty<Port>(), Port.ReturnPort(NumType.Instance)) =>
            _value = value;

        public override IValue Call(IValue[] _, CompilationContext __) => new Constant(_value switch
        {
            Value.NaN => float.NaN,
            Value.PositiveInfinity => float.PositiveInfinity,
            Value.NegativeInfinity => float.NegativeInfinity,
            _ => throw new InternalCompilerException("Intrinsic nullary case not handled")
        });
    }
}