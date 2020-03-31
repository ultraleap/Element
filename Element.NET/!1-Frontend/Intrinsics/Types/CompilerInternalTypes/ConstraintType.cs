namespace Element.AST
{
    public sealed class ConstraintType : IType
    {
        private ConstraintType() {}
        public static IType Instance { get; } = new ConstraintType();
        public IType Type => TypeType.Instance;
        public string Name => "Constraint";
        public ISerializer? Serializer => null;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
    }
}