using System.Collections.Generic;

namespace Element.AST
{
    public sealed class TupleType : IntrinsicType
    {
        private TupleType() {}
        public static TupleType Instance { get; } = new TupleType();
        public override Result<IValue> Construct(StructDeclaration structDeclaration, IReadOnlyList<IValue> arguments, CompilationContext context) => new StructInstance(structDeclaration, arguments);
        public override Result<IValue> DefaultValue(CompilationContext context) => context.Trace(MessageCode.TypeError, "Cannot create a default value for Tuple");
        public override Result<bool> MatchesConstraint(StructDeclaration decl, IValue value, CompilationContext context) => value.FullyResolveValue(context).Map(v => v is StructInstance);
        public override Identifier Identifier { get; } = new Identifier("Tuple");
    }
}