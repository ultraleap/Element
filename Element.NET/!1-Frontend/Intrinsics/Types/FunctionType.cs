namespace Element.AST
{
    public class FunctionType : IType
    {
        private FunctionType() {}
        public static FunctionType Instance { get; } = new FunctionType();
        public string Name { get; } = "Function";
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value is IFunction;
    }
}