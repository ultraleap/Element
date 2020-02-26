namespace Element.AST
{
    public class NamespaceType : IType
    {
        private NamespaceType() {}
        public static NamespaceType Instance { get; } = new NamespaceType();
        public string Name { get; } = "Namespace";
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value is Namespace;
    }
}