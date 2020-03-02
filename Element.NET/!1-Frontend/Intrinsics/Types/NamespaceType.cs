namespace Element.AST
{
    public class NamespaceType : IType
    {
        private NamespaceType() {}
        public static NamespaceType Instance { get; } = new NamespaceType();
        public string Name => "Namespace";
        public IType Type => TypeType.Instance;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value is DeclaredNamespace;
    }
}