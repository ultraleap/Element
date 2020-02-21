namespace Element.AST
{
    public abstract class IntrinsicFunction : Intrinsic, ICallable
    {
        public IConstraint Identity { get; } = null; // TODO: Add function identity
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
    }
}