namespace Element.AST
{
    public interface IConstraint
    {
        bool? MatchesConstraint(IValue value, Port port, CompilationContext compilationContext);
    }
}