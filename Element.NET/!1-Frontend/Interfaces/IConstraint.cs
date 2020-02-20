namespace Element.AST
{
    public interface IConstraint : IValue
    {
        bool MatchesConstraint(IValue value);
    }
}