namespace Element.AST
{
    public class StructType : IType
    {
        private StructType() {}
        public static StructType Instance { get; } = new StructType();
        public string Name { get; } = "Struct";
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value is DeclaredStruct;
    }
}