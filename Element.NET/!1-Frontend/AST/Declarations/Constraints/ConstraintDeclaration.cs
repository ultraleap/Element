namespace Element.AST
{
    public abstract class ConstraintDeclaration : Declaration, IConstraint
    {
        protected override string Qualifier { get; } = "constraint";
        public override string ToString() => $"{Location}:Constraint";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Terminal)};
        public abstract Result<bool> MatchesConstraint(IValue value, CompilationContext context);
    }
}