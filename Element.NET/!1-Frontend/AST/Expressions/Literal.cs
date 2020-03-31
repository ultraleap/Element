using System.Globalization;
using Lexico;

namespace Element.AST
{
    public struct Literal : IScope, IValue
    {
        public Literal(float value) => Value = value;

        // ReSharper disable once AutoPropertyCanBeMadeGetOnly.Local
        [Term] public float Value { get; private set; }
        public static implicit operator float(Literal l) => l.Value;
        
        public override string ToString() => Value.ToString(CultureInfo.CurrentCulture);
        
        public IType Type => NumType.Instance;
        
        public IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
            NumType.Instance.ResolveInstanceFunction(id, this, compilationContext);
    }
}