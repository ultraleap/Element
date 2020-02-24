namespace Element.AST
{
    public interface IValue : IScopeItem
    {
        string TypeIdentity { get; }
    }
}