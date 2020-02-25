namespace Element.AST
{
    public abstract class IntrinsicStruct : Intrinsic, IConstructor<IType>, IType // Intrinsic structs don't need to be IConstraint or IScope - Declared structs handle all implementation of both.
    {
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
        public abstract IValue Call(IValue[] arguments, IType instanceType, CompilationContext compilationContext);
    }
}