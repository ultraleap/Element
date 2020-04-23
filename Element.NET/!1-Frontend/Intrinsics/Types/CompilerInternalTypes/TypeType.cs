namespace Element.AST
{
    public sealed class TypeType : IType
    {
        private TypeType() {}
        public static IType Instance { get; } = new TypeType();
        string IType.Name => "Type";
        IType IValue.Type => this;
        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
    }
}