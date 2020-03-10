using System.Globalization;
using Lexico;

namespace Element.AST
{
    public struct Literal : ISerializable, IScope, IValue
    {
        public Literal(float value)
        {
            Value = value;
            _declaringStruct = null;
        }

        // ReSharper disable once AutoPropertyCanBeMadeGetOnly.Local
        [Term] public float Value { get; private set; }
        public static implicit operator float(Literal l) => l.Value;
        public override string ToString() => Value.ToString(CultureInfo.CurrentCulture);

        public IType Type => NumType.Instance;

        public int SerializedSize => 1;
        public bool Serialize(ref float[] array, ref int position)
        {
            array[position] = Value;
            position++;
            return true;
        }

        private DeclaredStruct? _declaringStruct;

        public IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
            (_declaringStruct ??= (DeclaredStruct)compilationContext.GlobalScope[new Identifier(Type.Name), false, compilationContext]) switch
            {
                DeclaredStruct declaredStruct => declaredStruct.ResolveInstanceFunction(id, this, compilationContext),
                _ => compilationContext.LogError(7, $"Couldn't find instance function '{Type.Name}'")
            };
    }
}