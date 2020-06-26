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

            builder.Append(IntrinsicCache.Get<IntrinsicType>(Identifier, builder.Trace));
        }

        private IntrinsicType Implementation
        {
            get
            {
                var intrinsic = IntrinsicCache.Get<IntrinsicType>(Identifier, null);
                if (intrinsic.IsSuccess) return intrinsic.ResultOr(default!); // Guaranteed to return result as we checked first
                throw new InternalCompilerException($"No intrinsic '{Identifier.Value}' - this exception can only occur if validation is skipped");
            }
        }
        
        public override IReadOnlyList<Port> Fields => Implementation.Fields;
        public override Result<ISerializableValue> DefaultValue(CompilationContext context) => Implementation.DefaultValue(context);
        public override Result<IValue> Call(IReadOnlyList<IValue> fields, CompilationContext context) => Implementation.Construct(fields, context);
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => Implementation.MatchesConstraint(value, context);
    }
}