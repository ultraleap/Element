namespace Element.AST
{
    public sealed class FunctionType : IType
    {
        private FunctionType() {}
        public static IType Instance { get; } = new FunctionType();
        public static TypeAnnotation Annotation { get; } = new TypeAnnotation(Instance);
        public string Name => "Function";
        public IType Type => TypeType.Instance;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
    }
}