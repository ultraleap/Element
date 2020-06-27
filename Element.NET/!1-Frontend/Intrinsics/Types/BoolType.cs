using System.Collections.Generic;

namespace Element.AST
{
    public sealed class BoolType : IntrinsicType
    {
        private BoolType() { }
        public static BoolType Instance { get; } = new BoolType();
        public override Result<IValue> Construct(StructDeclaration decl, IReadOnlyList<IValue> arguments, CompilationContext context) =>
            IfIntrinsic.Instance.Call(new IValue[]{new Binary(Binary.Op.Gt, (Element.Expression)arguments[0], Constant.Zero),
                                      Constant.True,
                                      Constant.False}, context);
        public override Result<IValue> DefaultValue(CompilationContext _) => Constant.False;
        public override Result<bool> MatchesConstraint(StructDeclaration decl, IValue value, CompilationContext context) => value is Element.Expression expr && expr.Type == Instance;
        public override Identifier Identifier { get; } = new Identifier("Bool");
    }
}