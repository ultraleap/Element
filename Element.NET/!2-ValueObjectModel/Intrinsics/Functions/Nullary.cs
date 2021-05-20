using System;
using System.Collections.Generic;
using System.Linq;
using ResultNET;

namespace Element.AST
{
    public class Nullary : IntrinsicValue, IIntrinsicFunctionImplementation
    {
        static Nullary()
        {
            Instances = Enum.GetValues(typeof(Constant.Intrinsic))
                            .Cast<Constant.Intrinsic>()
                            .Select(v => new Nullary(v))
                            .ToDictionary(i => i._value);
        }
        
        public static IReadOnlyDictionary<Constant.Intrinsic, Nullary> Instances { get; }
        
        private readonly Constant.Intrinsic _value;
        
        private Nullary(Constant.Intrinsic value)
        {
            _value = value;
            Identifier = new Identifier(value.ToString());
        }

        public override Identifier Identifier { get; }
        public bool IsVariadic => false;
        public override Result<IValue> Call(IReadOnlyList<IValue> _, Context __) =>
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