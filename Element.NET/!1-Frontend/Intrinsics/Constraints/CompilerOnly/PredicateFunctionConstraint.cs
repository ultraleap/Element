namespace Element.AST
{
    public class PredicateFunctionConstraint : IConstraint
    {
        private PredicateFunctionConstraint() {}
        public static PredicateFunctionConstraint Instance { get; } = new PredicateFunctionConstraint();
        Result<bool> IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            value is IFunctionSignature fn
                ? fn.Output.ResolveConstraint(compilationContext)
                    .Bind(constraint => BoolType.Instance.Declaration(compilationContext.SourceContext)
                                                .Map(boolDeclaration => constraint == boolDeclaration))
                : false;
    }
}