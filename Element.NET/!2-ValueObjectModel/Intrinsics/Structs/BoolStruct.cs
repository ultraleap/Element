using System.Collections.Generic;

namespace Element.AST
{
    public sealed class BoolStruct : IIntrinsicStructImplementation
    {
        private BoolStruct() { }
        public static BoolStruct Instance { get; } = new BoolStruct();
        public Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, Context context) =>
            Element.Binary.CreateAndOptimize(Element.Binary.Op.Gt, arguments[0], Constant.Zero, context)
                   .Bind(condition => If.Instance.Call(new[]{condition, Constant.True, Constant.False}, context));
        public Result<IValue> DefaultValue(Context _) => Constant.False;

        public Result<bool> MatchesConstraint(Struct @struct, IValue value, Context context) => value.IsType<Instruction>(out var instruction) && instruction.StructImplementation == Instance;
        public Identifier Identifier { get; } = new Identifier("Bool");
    }
}