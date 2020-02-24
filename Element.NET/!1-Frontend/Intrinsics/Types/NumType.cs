namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IntrinsicStruct
    {
        private NumType() {}
        public static NumType Instance { get; } = new NumType();
        public static string TypeIdentity { get; } = "Num";
        public override string FullPath => TypeIdentity;
        public override string ToString() => "<number>";
        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) => Call(arguments, FullPath, compilationContext);
        public override IValue Call(IValue[] arguments, string instanceTypeIdentity, CompilationContext compilationContext) =>
            !arguments.ValidateArgumentCount(1, compilationContext) ? CompilationErr.Instance :
            arguments[0] is Literal lit ? (IValue) new Literal(lit, instanceTypeIdentity) :
            compilationContext.LogError(8, "Argument must be a number");
    }
}