using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class BoolType : IntrinsicType
    {
        private BoolType() => Fields = new[] {new Port("a", NumType.Instance)};
        public static BoolType Instance { get; } = new BoolType();
        public override Port[] Fields { get; }
        public override Result<IValue> Construct(StructDeclaration declaration, IEnumerable<IValue> arguments) =>
            IfIntrinsic.Instance.Call(new IValue[]{new Binary(Binary.Op.Gt, (Element.Expression)arguments.First(), Constant.Zero),
                                      Constant.True,
                                      Constant.False});
        public override Result<ISerializableValue> DefaultValue(CompilationContext _) => Constant.False;
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext compilationContext) => value is Element.Expression expr && expr.Type == Instance;
        public override Identifier Identifier { get; } = new Identifier("Bool");
    }
}