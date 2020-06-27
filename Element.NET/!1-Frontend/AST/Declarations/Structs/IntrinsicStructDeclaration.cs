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

        public IntrinsicType ImplementingIntrinsic
        {
            get
            {
                var intrinsic = IntrinsicCache.Get<IntrinsicType>(Identifier, null);
                if (intrinsic.IsSuccess) return intrinsic.ResultOr(default!); // Guaranteed to return result as we checked first
                throw new InternalCompilerException($"No intrinsic '{Identifier.Value}' - this exception can only occur if validation is skipped");
            }
        }
        
        public override Result<IValue> DefaultValue(CompilationContext context) => ImplementingIntrinsic.DefaultValue(context);
        public override Result<IValue> Call(IReadOnlyList<IValue> fields, CompilationContext context) => ImplementingIntrinsic.Construct(this, fields, context);
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => ImplementingIntrinsic.MatchesConstraint(this, value, context);
    }
}