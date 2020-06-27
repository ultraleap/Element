namespace Element.AST
{
    /// <summary>
    /// A function type that accepts unary (1-arity) functions.
    /// </summary>
    public sealed class UnaryFunctionConstraint : Value
    {
        private UnaryFunctionConstraint() { }
        public static UnaryFunctionConstraint Instance { get; } = new UnaryFunctionConstraint();
        public override string ToString() => "<unary function constraint>";
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) =>
            value.FullyResolveValue(context).Map(v => v is IFunctionSignature fn && fn.Inputs.Count == 1);
    }
}