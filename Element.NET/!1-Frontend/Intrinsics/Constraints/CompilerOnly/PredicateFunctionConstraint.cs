namespace Element.AST
{
    public class PredicateFunctionConstraint : Value
    {
        private PredicateFunctionConstraint() {}
        public static PredicateFunctionConstraint Instance { get; } = new PredicateFunctionConstraint();
        public override string ToString() => "<predicate function constraint>";
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) =>
            value.FullyResolveValue(context).Bind(v => v is IFunctionSignature fn
                                                           ? fn.Output.ResolveConstraint(context)
                                                               .Map(constraint => constraint.IsIntrinsicType<BoolType>())
                                                           : false);
    }
}