namespace Element.AST
{
    public sealed class TupleType : IntrinsicType
    {
        public static TupleType Instance { get; } = new TupleType();
        protected override IntrinsicType _instance => Instance;
        public override string Name => "Tuple";
        public override Port[] Inputs { get; } = {Port.VariadicPort};
    }
}