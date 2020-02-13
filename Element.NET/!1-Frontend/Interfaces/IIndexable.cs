namespace Element.AST
{
    public interface IIndexable : IValue
    {
        IValue? this[Identifier id] { get; }
    }
}