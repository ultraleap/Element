using System.Globalization;
using Lexico;

namespace Element.AST
{
    public class Literal : IExpressionListStart, IValue, ISerializable
    {
        // ReSharper disable once UnusedMember.Global - Used by Lexico to construct instances
        public Literal() {}
        public Literal(float value) {Value = value;}
        public Literal(float value, IType instanceType) :this(value) {Type = instanceType;}

        // ReSharper disable once AutoPropertyCanBeMadeGetOnly.Local
        [Term] public float Value { get; private set; }
        public static implicit operator float(Literal l) => l.Value;
        public override string ToString() => Value.ToString(CultureInfo.CurrentCulture);
        public IType Type { get; } = NumType.Instance;
        public int SerializedSize => 1;
        public bool Serialize(ref float[] array, ref int position)
        {
            array[position] = Value;
            position++;
            return true;
        }
    }
}