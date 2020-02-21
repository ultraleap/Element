namespace Element.AST
{
    public interface ICallable
    {
        IValue Call(IValue[] arguments, CompilationContext compilationContext);
    }

    public interface IConstructor : ICallable
    {
        IValue Call(IValue[] arguments, IConstraint instanceIdentity, CompilationContext compilationContext);
    }
}