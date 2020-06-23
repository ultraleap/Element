namespace Element.AST
{
    public interface IType : IConstraint
    {
        Result<ISerializableValue> DefaultValue(CompilationContext compilationContext);
    }
}