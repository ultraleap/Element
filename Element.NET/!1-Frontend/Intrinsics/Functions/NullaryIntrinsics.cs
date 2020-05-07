using System;

namespace Element.AST
{
    public class NullaryIntrinsics : IntrinsicFunction
    {
        private readonly Constant.Intrinsic _value;
        
        public NullaryIntrinsics(Constant.Intrinsic value)
            : base(value switch
            {
                { } v when v == Constant.Intrinsic.True || v == Constant.Intrinsic.False => v.ToString(),
                _ => $"Num.{value.ToString()}"
            }, Array.Empty<Port>(), Port.ReturnPort(NumType.Instance))
            => _value = value;

        public override IValue Call(IValue[] _, CompilationContext __) =>
            _value switch
            {
                Constant.Intrinsic.True => Constant.True,
                Constant.Intrinsic.False => Constant.False,
                Constant.Intrinsic.NaN => Constant.NaN,
                Constant.Intrinsic.PositiveInfinity => Constant.PositiveInfinity,
                Constant.Intrinsic.NegativeInfinity => Constant.NegativeInfinity,
                _ => throw new InternalCompilerException("Intrinsic nullary case not handled")
            };
    }
}