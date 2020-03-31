namespace Element.AST
{
    public abstract class IntrinsicType<TType> : IIntrinsic, IFunction, IType where TType : IntrinsicType<TType>, new()
    {
        protected IntrinsicType() {}
        public static TType Instance { get; } = new TType();
        
        public IType Type => TypeType.Instance;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
        public string Location => Name;
        public abstract Port[] Inputs { get; }
        public Port Output { get; } = Port.ReturnPort(Instance);
        public abstract string Name { get; }
        public abstract ISerializer? Serializer { get; }
    }
}