using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IntrinsicType
    {
        private NumType() => Fields = new[]{new Port("a", Instance)};
        public static NumType Instance { get; } = new NumType();
        public override Port[] Fields { get; }
        public override Result<IValue> Construct(StructDeclaration declaration, IEnumerable<IValue> arguments) => new Result<IValue>(arguments.First());
        public override Result<ISerializableValue> DefaultValue(CompilationContext _) => Constant.Zero;
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext compilationContext) => value is Element.Expression expr && expr.Type == Instance;
        public override Identifier Identifier { get; } = new Identifier("Num");
    }
}