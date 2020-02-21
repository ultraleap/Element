namespace Element.AST
{
    public abstract class IntrinsicConstraint : Intrinsic, IConstraint
    {
        public abstract bool MatchesConstraint(IValue value, CompilationContext compilationContext);
    }
}