using System.Collections.Generic;

namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public sealed class IntrinsicStructDeclaration : StructDeclaration
    {
        protected override string IntrinsicQualifier => "intrinsic";

        protected override void AdditionalValidation(ResultBuilder builder)
        {
            if (DeclaredType != null)
            {
                builder.Append(MessageCode.StructCannotHaveReturnType, $"Struct '{Identifier}' cannot have declared return type");
            }
        }

        public Result<IIntrinsicType> ImplementingIntrinsic =>
            _idToIntrinsic.TryGetValue(Identifier, out var intrinsicType)
                ? new Result<IIntrinsicType>(intrinsicType)
                : (MessageCode.IntrinsicNotFound, $"Intrinsic '{Identifier}' not found");

        private static readonly Dictionary<string, IIntrinsicType> _idToIntrinsic = new Dictionary<string, IIntrinsicType>
        {
            {"Num", NumType.Instance},
            {"Bool", BoolType.Instance},
            {"List", ListType.Instance},
            {"Tuple", TupleType.Instance}
        };

        public override Result<Port[]> Fields => ImplementingIntrinsic.Bind(type => Fields);
        public override Result<ISerializableValue> DefaultValue(CompilationContext compilationContext) => ImplementingIntrinsic.Bind(type => type.DefaultValue);
        protected override Result<IValue> Construct(IEnumerable<IValue> arguments) => ImplementingIntrinsic.Bind(type => type.Construct(this, arguments));
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext compilationContext) => ImplementingIntrinsic.Bind(type => type.MatchesConstraint(value, compilationContext));
    }
}