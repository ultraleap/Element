using System.Collections.Generic;

namespace Element.AST
{
    public sealed class TupleType : IntrinsicType
    {
        private TupleType() => Fields = new[] {Port.VariadicPort};
        public static TupleType Instance { get; } = new TupleType();
        public override IReadOnlyList<Port> Fields { get; }
        public override Result<IValue> Construct(IReadOnlyList<IValue> arguments, CompilationContext context) =>
            Declaration(context.SourceContext).Map(decl => (IValue)new StructInstance(decl, arguments));
        public override Result<ISerializableValue> DefaultValue(CompilationContext context) => context.Trace(MessageCode.TypeError, "Cannot create a default value for Tuple");
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => value.FullyResolveValue(context).Map(v => v is StructInstance);
        public override Identifier Identifier { get; } = new Identifier("Tuple");
    }
}