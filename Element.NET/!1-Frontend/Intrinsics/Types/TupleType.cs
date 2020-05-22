namespace Element.AST
{
    public sealed class TupleType : IIntrinsicType
    {
        private TupleType()
        {
            Inputs = new[] {Port.VariadicPort};
            Output = Port.ReturnPort(AnyConstraint.Instance);
        }
        public static TupleType Instance { get; } = new TupleType();
        public Port[] Inputs { get; }
        public Port Output { get; }
        public IValue Call(IValue[] arguments, CompilationContext compilationContext) => Declaration(compilationContext).CreateInstance(arguments);
        string IIntrinsic.Location => "Tuple";
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => Declaration(compilationContext);

        private IntrinsicStructDeclaration Declaration(CompilationContext context) =>
            context.GetIntrinsicsDeclaration<IntrinsicStructDeclaration>(this);

        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value is StructInstance;

        public ISerializableValue DefaultValue(CompilationContext context) =>
            context.LogError(14, $"Cannot create a default value for Tuple");
    }
}