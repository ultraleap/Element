namespace Element.AST
{
    public sealed class NamespaceType : IType
    {
        private NamespaceType() {}
        public static NamespaceType Instance { get; } = new NamespaceType();
        public string Name => "Namespace";
        public ISerializer? Serializer => null;
        public IType Type => TypeType.Instance;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
    }
}