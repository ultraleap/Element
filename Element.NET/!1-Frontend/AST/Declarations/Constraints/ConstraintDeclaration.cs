namespace Element.AST
{
    public abstract class ConstraintDeclaration : Declaration, IConstraint
    {
        protected override string Qualifier { get; } = "constraint";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Terminal)};
        public abstract bool MatchesConstraint(IValue value, CompilationContext compilationContext);
    }
}