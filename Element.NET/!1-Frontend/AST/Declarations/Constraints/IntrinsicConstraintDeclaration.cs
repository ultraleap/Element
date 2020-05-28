namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class IntrinsicConstraintDeclaration : ConstraintDeclaration
    {
        protected override string IntrinsicQualifier { get; } = "intrinsic";

        protected override bool AdditionalValidation(SourceContext sourceContext) => ImplementingIntrinsic<IConstraint>(sourceContext) != null;

        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            ImplementingIntrinsic<IConstraint>(compilationContext)?.MatchesConstraint(value, compilationContext) ?? false;
    }
}