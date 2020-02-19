namespace Element.AST
{
    public interface IConstraint : IValue
    {
        bool? MatchesConstraint(IValue value, Port port, CompilationContext compilationContext);
    }
}