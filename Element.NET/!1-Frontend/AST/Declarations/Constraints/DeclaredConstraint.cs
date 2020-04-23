namespace Element.AST
{
    public abstract class DeclaredConstraint : Declaration, IConstraint
    {
        protected override string Qualifier { get; } = "constraint";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Terminal)};
        public override IType Type => ConstraintType.Instance;
        public abstract bool MatchesConstraint(IValue value, CompilationContext compilationContext);
    }
}