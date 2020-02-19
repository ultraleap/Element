namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IConstraint
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
    }
}