namespace Element.AST
{
    public class PredicateFunctionConstraint : IConstraint
    {
        private PredicateFunctionConstraint() {}
        public static PredicateFunctionConstraint Instance { get; } = new PredicateFunctionConstraint();
        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            value is IFunctionSignature fn
            && fn.Output.ResolveConstraint(compilationContext.SourceContext.GlobalScope, compilationContext) == BoolType.Instance.GetIntrinsicsDeclaration<Declaration>(compilationContext);
    }
}