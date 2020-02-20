namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IConstraint, ICallable
    {
        public static IConstraint Instance { get; } = new NumType();
        private NumType() { }
        public override string ToString() => "<number>";

        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value switch
        {
            Literal _ => true,
            _ => false
        };

        public bool CanBeCached => true;

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            !arguments.ValidateArgumentCount(1, compilationContext) ? CompilationErr.Instance :
            arguments[0] is Literal lit ? (IValue) lit :
            compilationContext.LogError(8, "Argument must be a number");
    }
}