using System.Collections.Generic;

namespace Element.AST
{
    public sealed class TupleType : IntrinsicType
    {
        private TupleType() => Fields = new[] {Port.VariadicPort};
        public static TupleType Instance { get; } = new TupleType();
        public override Port[] Fields { get; }
        public override Result<IValue> Construct(StructDeclaration declaration, IEnumerable<IValue> arguments) => declaration.CreateInstance(arguments);
        public override Result<ISerializableValue> DefaultValue(CompilationContext _) => (MessageCode.TypeError, "Cannot create a default value for Tuple");
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext compilationContext) => value is StructInstance;

        public override Identifier Identifier { get; } = new Identifier("Tuple");
    }
}