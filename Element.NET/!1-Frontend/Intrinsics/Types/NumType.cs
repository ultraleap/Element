using System.Collections.Generic;

namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IntrinsicType
    {
        private NumType() { }
        public static NumType Instance { get; } = new NumType();
        public override Result<IValue> Construct(StructDeclaration decl, IReadOnlyList<IValue> arguments, CompilationContext context) => new Result<IValue>(arguments[0]);
        public override Result<IValue> DefaultValue(CompilationContext _) => Constant.Zero;
        public override Result<bool> MatchesConstraint(StructDeclaration decl, IValue value, CompilationContext context) => value.FullyResolveValue(context).Map(v => v is Element.Expression expr && expr.Type == Instance);
        public override Identifier Identifier { get; } = new Identifier("Num");
    }
}