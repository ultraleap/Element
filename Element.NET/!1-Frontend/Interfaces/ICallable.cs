namespace Element.AST
{
    public interface ICallable : IValue
    {
        IValue Call(CompilationFrame frame, CompilationContext compilationContext);
        Port[] Inputs { get; }
    }
}