using System.Globalization;
using Lexico;

namespace Element.AST
{
    public class Literal : IExpressionListStart, IValue
    {
        public Literal() {} // Need parameterless constructor for Lexico to construct instance
        public Literal(float value) {Value = value;}
        public Literal(float value, IType instanceType) :this(value) {Type = instanceType;}

        [field: Term] public float Value { get; }
        public static implicit operator float(Literal l) => l.Value;
        public override string ToString() => Value.ToString(CultureInfo.CurrentCulture);
        public IType Type { get; } = NumType.Instance;


    }
}