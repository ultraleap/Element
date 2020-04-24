namespace Element.AST
{
    public abstract class IntrinsicType : IIntrinsic, IFunction, IType
    {
        protected abstract IntrinsicType _instance { get; }
        IType IValue.Type => TypeType.Instance;
        public virtual bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == _instance;
        public virtual IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            compilationContext.GetIntrinsicsDeclaration<DeclaredStruct>(this)
                              ?.CreateInstance(RefineArguments(arguments, compilationContext), _instance);
        protected virtual IValue[] RefineArguments(IValue[] arguments, CompilationContext _) => arguments;
        string IIntrinsic.Location => Name;
        public abstract string Name { get; }
        public abstract Port[] Inputs { get; }
        Port IFunctionSignature.Output => Port.ReturnPort(_instance);
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => compilationContext.GetIntrinsicsDeclaration<IntrinsicStructDeclaration>(_instance);
    }
}