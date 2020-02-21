namespace Element.AST
{
    public abstract class IntrinsicStruct : Intrinsic, IConstructor, IConstraint // Intrinsic structs don't need to be IScope - Declared structs handle all scope implementation.
    {
        public abstract IConstraint Identity { get; }
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
        public abstract IValue Call(IValue[] arguments, IConstraint instanceIdentity, CompilationContext compilationContext);
        public abstract bool MatchesConstraint(IValue value, CompilationContext compilationContext);
    }
}