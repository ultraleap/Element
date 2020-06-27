namespace Element.AST
{
    /// <summary>
    /// A constraint that accepts any function.
    /// </summary>
    public sealed class FunctionConstraint : Value
    {
        private FunctionConstraint() {}
        public static FunctionConstraint Instance { get; } = new FunctionConstraint();
        public override string ToString() => "<function constraint>";
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => value.FullyResolveValue(context).Map(v => v is IFunctionSignature);
    }
}