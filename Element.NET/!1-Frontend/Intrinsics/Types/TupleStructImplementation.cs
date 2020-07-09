using System.Collections.Generic;

namespace Element.AST
{
    public sealed class TupleStructImplementation : IntrinsicStructImplementation
    {
        private TupleStructImplementation() {}
        public static TupleStructImplementation Instance { get; } = new TupleStructImplementation();
        public override Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, CompilationContext context) => new StructInstance(@struct, arguments);
        public override Result<IValue> DefaultValue(CompilationContext context) => context.Trace(MessageCode.TypeError, "Cannot create a default value for Tuple");
        public override Result<bool> MatchesConstraint(Struct @struct, IValue value, CompilationContext context) => value.FullyResolveValue(context).Map(v => v is StructInstance);
        public override Identifier Identifier { get; } = new Identifier("Tuple");
    }
}