using System;

namespace Element.AST
{
    public class NullaryIntrinsics : IIntrinsic, IFunction
    {
        private readonly Value _value;

        public enum Value
        {
            NaN,
            PositiveInfinity,
            NegativeInfinity,
        }

        public NullaryIntrinsics(Value value)
        {
            Location = $"Num.{value.ToString()}";
            _value = value;
        }

        public IType Type => FunctionType.Instance;

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) => new Literal(_value switch
        {
            Value.NaN => float.NaN,
            Value.PositiveInfinity => float.PositiveInfinity,
            Value.NegativeInfinity => float.NegativeInfinity,
            _ => throw new InternalCompilerException("Intrinsic nullary case not handled")
        });

        public Port[] Inputs { get; } = Array.Empty<Port>();
        public Port Output { get; } = Port.ReturnPort(NumType.Instance);
        public string Location { get; }
    }
}