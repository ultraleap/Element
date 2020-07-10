using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public class NullaryIntrinsicsFunctionImplementation : IntrinsicFunctionImplementation
    {
        static NullaryIntrinsicsFunctionImplementation()
        {
            Instances = Enum.GetValues(typeof(Constant.Intrinsic))
                            .Cast<Constant.Intrinsic>()
                            .Select(v => new NullaryIntrinsicsFunctionImplementation(v))
                            .ToDictionary(i => i._value);
        }
        
        public static IReadOnlyDictionary<Constant.Intrinsic, NullaryIntrinsicsFunctionImplementation> Instances { get; }
        
        private readonly Constant.Intrinsic _value;
        
        private NullaryIntrinsicsFunctionImplementation(Constant.Intrinsic value)
        {
            _value = value;
            Identifier = new Identifier(value.ToString());
        }

        public override Identifier Identifier { get; }

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