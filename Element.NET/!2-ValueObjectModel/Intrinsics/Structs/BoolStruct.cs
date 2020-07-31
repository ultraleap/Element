using System.Collections.Generic;

namespace Element.AST
{
    public sealed class BoolStruct : IIntrinsicStructImplementation
    {
        private BoolStruct() { }
        public static BoolStruct Instance { get; } = new BoolStruct();
        public Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, Context context) =>
            If.Instance.Call(new IValue[]{Element.Binary.CreateAndOptimize(Element.Binary.Op.Gt, (Element.Expression)arguments[0], Constant.Zero),
                Constant.True,
                Constant.False}, context);
        public Result<IValue> DefaultValue(Context _) => Constant.False;

        public Result<bool> MatchesConstraint(Struct @struct, IValue value, Context context) => value is Element.Expression expr && expr.StructImplementation == Instance;
        public Identifier Identifier { get; } = new Identifier("Bool");
    }
}