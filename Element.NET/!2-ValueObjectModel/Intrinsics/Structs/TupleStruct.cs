using System.Collections.Generic;

namespace Element.AST
{
    public sealed class TupleStruct : IIntrinsicStructImplementation
    {
        private TupleStruct() {}
        public static TupleStruct Instance { get; } = new TupleStruct();
        public Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, CompilationContext context) => StructInstance.Create(@struct, arguments, context).Cast<IValue>(context);
        public Result<IValue> DefaultValue(CompilationContext context) => context.Trace(MessageCode.TypeError, "Cannot create a default value for Tuple");
        public Result<bool> MatchesConstraint(Struct @struct, IValue value, CompilationContext context) => value.FullyResolveValue(context).Map(v => v is StructInstance);
        public Identifier Identifier { get; } = new Identifier("Tuple");
    }
}