namespace Element.AST
{
    public abstract class IntrinsicFunction : Intrinsic, ICallable
    {
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
    }
}