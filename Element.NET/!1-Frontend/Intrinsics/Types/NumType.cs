namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IntrinsicStruct
    {
        private NumType() {}
        public static NumType Instance { get; } = new NumType();
        public override string FullPath { get; } = "Num";
        public override string ToString() => "<number>";
        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value switch
        {
            Literal _ => true,
            _ => false
        };
        public override IConstraint Identity => Instance;
        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) => Call(arguments, Instance, compilationContext);
        public override IValue Call(IValue[] arguments, IConstraint instanceIdentity, CompilationContext compilationContext) =>
            !arguments.ValidateArgumentCount(1, compilationContext) ? CompilationErr.Instance :
            arguments[0] is Literal lit ? (IValue) new Literal(lit, instanceIdentity) :
            compilationContext.LogError(8, "Argument must be a number");
    }
}