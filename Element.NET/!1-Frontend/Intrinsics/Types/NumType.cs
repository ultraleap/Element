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

        public bool MatchesConstraint(IValue value) => value switch
        {
            Literal _ => true,
            StructInstance i when i.Type == Instance => true,
            _ => false
        };

        public bool CanBeCached => true;
        public IValue Call(IValue[] arguments, CompilationContext compilationContext) => arguments[0] as Literal;

        public Port[] Inputs { get; } = {new Port(new Identifier("a"), new Type())};
    }
}