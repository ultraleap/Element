namespace Element.AST
{
    public class PredicateFunctionConstraint : IConstraint
    {
        private PredicateFunctionConstraint() {}
        public static PredicateFunctionConstraint Instance { get; } = new PredicateFunctionConstraint();
        Result<bool> IConstraint.MatchesConstraint(IValue value, CompilationContext context) =>
            value.FullyResolveValue(context).Bind(v => v is IFunction fn
                                                           ? fn.Output.ResolveConstraint(context)
                                                               .Bind(constraint => BoolType.Instance.Declaration(context.SourceContext)
                                                                                           .Map(boolDeclaration => constraint == boolDeclaration))
                                                           : false);
    }
}