namespace Element.AST
{
    /// <summary>
    /// A type that accepts any other type, e.g. value, expression, function. The default type in Element.
    /// </summary>
    public sealed class AnyType : IConstraint
    {
        public static IConstraint Instance { get; } = new AnyType();
        private AnyType() { }
        public override string ToString() => "<any>";
        bool? IConstraint.MatchesConstraint(IValue value, Port port, CompilationContext compilationContext) => true;
    }
}