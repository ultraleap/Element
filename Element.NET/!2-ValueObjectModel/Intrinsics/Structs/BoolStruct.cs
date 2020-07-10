using System.Collections.Generic;

namespace Element.AST
{
    public sealed class BoolStruct : IntrinsicStructImplementation
    {
        private BoolStruct() { }
        public static BoolStruct Instance { get; } = new BoolStruct();
        public override Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, CompilationContext context) =>
            IfIntrinsicFunctionImplementation.Instance.Call(new IValue[]{new Binary(Binary.Op.Gt, (Element.Expression)arguments[0], Constant.Zero),
                                      Constant.True,
                                      Constant.False}, context);
        public override Result<IValue> DefaultValue(CompilationContext _) => Constant.False;
        public override Result<bool> MatchesConstraint(Struct @struct, IValue value, CompilationContext context) => value is Element.Expression expr && expr.StructImplementation == Instance;
        public override Identifier Identifier { get; } = new Identifier("Bool");
    }
}