namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IntrinsicType<NumType>
    {
        public override Port[] Inputs { get; } = {new Port("a", Instance)};
        public override string Name => "Num";
        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) => arguments[0]; // Return the number - type checking will already have occurred.
    }
}