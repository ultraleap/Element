namespace Element.AST
{
    public interface IType : IConstraint
    {
        string Name { get; }
        ISerializer? Serializer { get; }
    }
}