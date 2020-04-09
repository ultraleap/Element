namespace Element.AST
{
    public sealed class FunctionType : IType
    {
        private FunctionType() {}
        public static IType Instance { get; } = new FunctionType();
        string IType.Name => "Function";
        IType IValue.Type => TypeType.Instance;
        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
    }
}