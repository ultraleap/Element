using System.Collections.Generic;

namespace Element.AST
{
    public sealed class BoolStructImplementation : IntrinsicStructImplementation
    {
        private BoolStructImplementation() { }
        public static BoolStructImplementation Instance { get; } = new BoolStructImplementation();
        public override Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, CompilationContext context) =>
            IfIntrinsicFunctionImplementation.Instance.Call(new IValue[]{new Binary(Binary.Op.Gt, (Element.Expression)arguments[0], Constant.Zero),
                                      Constant.True,
                                      Constant.False}, context);
        public override Result<IValue> DefaultValue(CompilationContext _) => Constant.False;
        public override Result<bool> MatchesConstraint(Struct @struct, IValue value, CompilationContext context) => value is Element.Expression expr && expr.StructImplementation == Instance;
        public override Identifier Identifier { get; } = new Identifier("Bool");
    }
}