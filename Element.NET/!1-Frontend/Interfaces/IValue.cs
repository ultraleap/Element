namespace Element.AST
{
    public interface IValue : IScopeItem
    {
        IConstraint Identity { get; }
    }
}