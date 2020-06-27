namespace Element.AST
{
    public abstract class ConstraintDeclaration : Declaration
    {
        protected override string Qualifier { get; } = "constraint";
        public override string ToString() => $"{Location}:Constraint";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Terminal)};
    }
}