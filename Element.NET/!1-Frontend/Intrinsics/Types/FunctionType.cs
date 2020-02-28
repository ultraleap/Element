namespace Element.AST
{
    public class FunctionType : IType
    {
        private FunctionType() {}
        public static IType Instance { get; } = new FunctionType();
        public string Name => "Function";
        public IType Type => TypeType.Instance;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value is IFunction;
    }
}