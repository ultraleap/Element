namespace Element.AST
{
    public interface IValue : IScopeItem
    {
        IType Type { get; }
    }
}