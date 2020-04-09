namespace Element.AST
{
    public sealed class NamespaceType : IType
    {
        private NamespaceType() {}
        public static NamespaceType Instance { get; } = new NamespaceType();
        string IType.Name => "Namespace";
        IType IValue.Type => TypeType.Instance;
        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
    }
}