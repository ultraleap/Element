using System.Collections.Generic;

namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IntrinsicType
    {
        private NumType() => Fields = new[]{new Port("a", Instance)};
        public static NumType Instance { get; } = new NumType();
        public override IReadOnlyList<Port> Fields { get; }
        public override Result<IValue> Construct(IReadOnlyList<IValue> arguments, CompilationContext context) => new Result<IValue>(arguments[0]);
        public override Result<ISerializableValue> DefaultValue(CompilationContext _) => Constant.Zero;
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => value.FullyResolveValue(context).Map(v => v is Element.Expression expr && expr.Type == Instance);
        public override Identifier Identifier { get; } = new Identifier("Num");
    }
}