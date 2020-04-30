namespace Element.AST
{
    public interface IIntrinsic : IValue
    {
        string Location { get; }
    }
}