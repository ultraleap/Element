namespace Element.AST
{
    public interface IConstraint : IValue
    {
        Result<bool> MatchesConstraint(IValue value, CompilationContext compilationContext);
    }
}