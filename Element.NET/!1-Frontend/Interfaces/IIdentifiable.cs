namespace Element.AST
{
    public interface IIdentifiable : IScopeItem
    {
        Identifier Identifier { get; }
    }
}