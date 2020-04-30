namespace Element.AST
{
    public class PredicateFunctionConstraint : IConstraint
    {
        private PredicateFunctionConstraint() {}
        public static PredicateFunctionConstraint Instance { get; } = new PredicateFunctionConstraint();
        public override string ToString() => "Predicate";
        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            value is IFunctionSignature fn
            && fn.Output.ResolveConstraint(compilationContext.SourceContext.GlobalScope, compilationContext) == BoolType.Instance.GetIntrinsicsDeclaration<Declaration>(compilationContext);
        IType IValue.Type => ConstraintType.Instance;
    }
}