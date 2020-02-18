namespace Element.AST
{
    public interface ICallable : IValue
    {
        IValue Call(IValue[] arguments, CompilationContext compilationContext);
        Port[] Inputs { get; }
    }
}