namespace Element.AST
{
    public sealed class TypeType : IType
    {
        private TypeType() {}
        public static IType Instance { get; } = new TypeType();
        public IType Type => this;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
        public string Name => "Type";
        public ISerializer? Serializer => null;
    }
}