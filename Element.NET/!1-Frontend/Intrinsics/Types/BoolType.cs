namespace Element.AST
{
    public sealed class BoolType : SerializableIntrinsicType
    {
        public static BoolType Instance { get; } = new BoolType();
        protected override IntrinsicType _instance => Instance;
        public override string Name => "Bool";
        public override Port[] Inputs { get; } = {new Port("a", NumType.Instance)};

        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            value is Element.Expression expr && expr.InstanceTypeOverride == Instance;

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            IntrinsicCache.GetByLocation<IFunctionSignature>("Bool.if", compilationContext).ResolveCall(new IValue[]
            {
                new Binary(Binary.Op.Gt, arguments[0] as Element.Expression, Constant.Zero),
                Constant.True,
                Constant.False
            }, false, compilationContext);

        public override int Size(IValue instance, CompilationContext compilationContext) => 1;

        public override bool Serialize(IValue instance, ref Element.Expression[] serialized, ref int position, CompilationContext compilationContext)
        {
            if (!(instance is Element.Expression expr && expr.InstanceTypeOverride == Instance)) return false;
            serialized[position++] = expr;
            return true;
        }
    }
}