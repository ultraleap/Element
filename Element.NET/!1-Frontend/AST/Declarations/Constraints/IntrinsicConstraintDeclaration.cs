namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class IntrinsicConstraintDeclaration : ConstraintDeclaration
    {
        protected override string IntrinsicQualifier { get; } = "intrinsic";

        protected override void AdditionalValidation(ResultBuilder builder)
        {
            builder.Append(IntrinsicCache.Get<IntrinsicConstraint>(Identifier, builder.Trace));
        }

        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => Implementation.MatchesConstraint(value, context);
        
        private IntrinsicConstraint Implementation
        {
            get
            {
                var intrinsic = IntrinsicCache.Get<IntrinsicConstraint>(Identifier, null);
                if (intrinsic.IsSuccess) return intrinsic.ResultOr(default!); // Guaranteed to return result as we checked first
                throw new InternalCompilerException($"No intrinsic '{Identifier.Value}' - this exception can only occur if validation is skipped");
            }
        }
    }
}