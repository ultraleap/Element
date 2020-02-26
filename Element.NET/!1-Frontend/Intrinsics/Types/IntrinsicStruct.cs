namespace Element.AST
{
    public abstract class IntrinsicStruct : Intrinsic, IConstructor<IType>, IType // Intrinsic structs don't need to be IScope - Declared structs handle all implementation.
    {
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
        public abstract IValue Call(IValue[] arguments, IType instanceType, CompilationContext compilationContext);
        public string Name => (Declarer as IType).Name;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            (Declarer as DeclaredStruct).MatchesConstraint(value, compilationContext);
    }
}