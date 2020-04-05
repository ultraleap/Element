namespace Element.AST
{
    public sealed class BoolType : IntrinsicType<BoolType>
    {
        public override string Name => "Bool";
        public override Port[] Inputs { get; } = {new Port("a", NumType.Instance)};
        protected override IValue[] RefineArguments(IValue[] arguments) => new IValue[] {new Constant((Constant)arguments[0] > 0f ? 1f : 0f)};
    }
}