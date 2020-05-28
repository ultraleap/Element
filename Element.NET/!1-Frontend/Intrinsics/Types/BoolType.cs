namespace Element.AST
{
    public sealed class BoolType : IIntrinsicType
    {
        private BoolType()
        {
            Inputs = new[] {new Port("a", NumType.Instance)};
            Output = Port.ReturnPort(this);
        }
        public static BoolType Instance { get; } = new BoolType();
        public string Location => "Bool";
        public ISerializableValue DefaultValue(CompilationContext _) => Constant.False;
        public Port[] Inputs { get; }
        public Port Output { get; }

        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            value is Element.Expression expr && expr.Type == Instance;

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            IntrinsicCache.GetByLocation<IFunctionSignature>("Bool.if", compilationContext)?.ResolveCall(new IValue[]
            {
                new Binary(Binary.Op.Gt, (Element.Expression) arguments[0], Constant.Zero),
                Constant.True,
                Constant.False
            }, false, compilationContext) ?? CompilationError.Instance;
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this.GetDeclaration(compilationContext);
    }
}