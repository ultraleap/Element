using System;
using System.Collections.Generic;

namespace Element.AST
{
    public class NullaryIntrinsics : IntrinsicFunction
    {
        private readonly Constant.Intrinsic _value;
        
        public NullaryIntrinsics(Constant.Intrinsic value)
        {
            _value = value;
            Identifier = new Identifier(value.ToString());
            Inputs = Array.Empty<Port>();
            Output = Port.ReturnPort(value switch
            {
                Constant.Intrinsic.True => BoolType.Instance,
                Constant.Intrinsic.False => BoolType.Instance,
                _ => NumType.Instance
            });
        }

        public override Identifier Identifier { get; }
        public override IReadOnlyList<Port> Inputs { get; }
        public override Port Output { get; }

        public override Result<IValue> Call(IReadOnlyList<IValue> _, CompilationContext __) =>
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