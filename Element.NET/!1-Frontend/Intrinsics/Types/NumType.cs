namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IntrinsicStruct
    {
        private NumType() { }
        public static NumType Instance { get; } = new NumType();
        public override string Location { get; } = "Num";
        public override string ToString() => "<number>";
        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) => Call(arguments, Instance, compilationContext);
        public override IValue Call(IValue[] arguments, IType instanceType, CompilationContext compilationContext) =>
            !arguments.ValidateArgumentCount(1, compilationContext) ? CompilationErr.Instance :
            arguments[0] is Literal lit ? (IValue) new Literal(lit, instanceType) :
            compilationContext.LogError(8, "Argument must be a number");
    }
}