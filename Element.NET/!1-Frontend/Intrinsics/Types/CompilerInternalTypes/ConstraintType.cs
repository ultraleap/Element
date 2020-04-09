namespace Element.AST
{
    public sealed class ConstraintType : IType
    {
        private ConstraintType() {}
        public static IType Instance { get; } = new ConstraintType();
        string IType.Name => "Constraint";
        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
        IType IValue.Type => TypeType.Instance;
    }
}