namespace Element.AST
{
    public abstract class IntrinsicStruct : Intrinsic, IConstructor // Intrinsic structs don't need to be IConstraint or IScope - Declared structs handle all implementation of both.
    {
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
        public abstract IValue Call(IValue[] arguments, string instanceTypeIdentity, CompilationContext compilationContext);
    }
}