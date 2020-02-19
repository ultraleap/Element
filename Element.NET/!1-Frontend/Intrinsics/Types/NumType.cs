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

        public bool? MatchesConstraint(IValue value, Port port, CompilationContext compilationContext) => value switch
        {
            Literal _ => true,
            _ => false
        };

        //value is CompilationErr ? (bool?)null : (value.Inputs?.Length == 0 && value.Outputs?.Length == 0);

        public bool CanBeCached => true;
        public IValue Call(IValue[] arguments, CompilationContext compilationContext) => arguments[0] as Literal;

        public Port[] Inputs { get; } = {new Port(new Identifier("a"), new Type())};
    }
}