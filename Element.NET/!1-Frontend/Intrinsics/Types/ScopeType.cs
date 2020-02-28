namespace Element.AST
{
    public class ScopeType : IType
    {
        private ScopeType(){}
        public static IType Instance { get; } = new ScopeType();
        public IType Type => TypeType.Instance;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value is IScope;
        public string Name => "Scope";
    }
}