namespace Element.AST
{
    public interface ICallable
    {
        IValue Call(IValue[] arguments, CompilationContext compilationContext);
    }

    public interface IConstructor<in TInstanceType> : ICallable where TInstanceType : IType
    {
        IValue Call(IValue[] arguments, TInstanceType instanceType, CompilationContext compilationContext);
    }
}