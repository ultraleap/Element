namespace Element.AST
{
    public interface IConstraint
    {
        bool MatchesConstraint(IValue value, CompilationContext compilationContext);
    }
}