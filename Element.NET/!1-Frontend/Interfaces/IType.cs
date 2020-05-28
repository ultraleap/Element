namespace Element.AST
{
    public interface IType : IConstraint
    {
        ISerializableValue DefaultValue(CompilationContext context);
    }
}