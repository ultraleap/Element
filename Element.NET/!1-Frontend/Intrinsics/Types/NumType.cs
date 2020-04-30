namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : SerializableIntrinsicType
    {
        public static NumType Instance { get; } = new NumType();
        protected override IntrinsicType _instance => Instance;
        public override Port[] Inputs { get; } = {new Port("a", Instance)};
        public override string Name => "Num";
        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) => arguments[0]; // Return the number - type checking will already have occurred.
        public override int Size(IValue instance, CompilationContext compilationContext) => 1;

        public override bool Serialize(IValue instance, ref Element.Expression[] serialized, ref int position, CompilationContext compilationContext)
        {
            if (!(instance is Element.Expression expr && expr.InstanceTypeOverride == null)) return false;
            serialized[position++] = expr;
            return true;
        }
    }
}