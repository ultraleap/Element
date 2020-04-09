namespace Element.AST
{
    public abstract class IntrinsicType<TType> : IIntrinsic, IFunction, IType where TType : IntrinsicType<TType>, new()
    {
        public static TType Instance { get; } = new TType();

        IType IValue.Type => TypeType.Instance;
        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
        public virtual IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            compilationContext.GetIntrinsicsDeclaration<DeclaredStruct>(this)
                              ?.CreateInstance(RefineArguments(arguments), Instance);
        protected virtual IValue[] RefineArguments(IValue[] arguments) => arguments;
        string IIntrinsic.Location => Name;
        public abstract string Name { get; }
        public abstract Port[] Inputs { get; }
        Port IFunctionSignature.Output { get; } = Port.ReturnPort(Instance);
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => compilationContext.GetIntrinsicsDeclaration<IntrinsicStructDeclaration>(Instance);
    }
}