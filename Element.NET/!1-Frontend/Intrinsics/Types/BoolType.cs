using System.Collections.Generic;

namespace Element.AST
{
    public sealed class BoolType : IntrinsicType
    {
        private BoolType() => Fields = new[] {new Port("a", NumType.Instance)};
        public static BoolType Instance { get; } = new BoolType();
        public override IReadOnlyList<Port> Fields { get; }
        public override Result<IValue> Construct(IReadOnlyList<IValue> arguments, CompilationContext context) =>
            IfIntrinsic.Instance.Call(new IValue[]{new Binary(Binary.Op.Gt, (Element.Expression)arguments[0], Constant.Zero),
                                      Constant.True,
                                      Constant.False}, context);
        public override Result<ISerializableValue> DefaultValue(CompilationContext _) => Constant.False;
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => value is Element.Expression expr && expr.Type == Instance;
        public override Identifier Identifier { get; } = new Identifier("Bool");
    }
}