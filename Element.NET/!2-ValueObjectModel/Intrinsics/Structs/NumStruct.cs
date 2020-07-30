using System.Collections.Generic;

namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumStruct : IIntrinsicStructImplementation
    {
        private NumStruct() { }
        public static NumStruct Instance { get; } = new NumStruct();
        public Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, CompilationContext context) => new Result<IValue>(arguments[0]);
        public Result<IValue> DefaultValue(CompilationContext _) => Constant.Zero;
        public Result<bool> MatchesConstraint(Struct @struct, IValue value, CompilationContext context) => value.FullyResolveValue(context).Map(v => v is Element.Expression expr && expr.StructImplementation == Instance);
        public Identifier Identifier { get; } = new Identifier("Num");
    }
}