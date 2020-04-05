namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class IntrinsicConstraint : DeclaredConstraint
    {
        protected override string IntrinsicQualifier { get; } = "intrinsic";

        internal override bool Validate(SourceContext sourceContext) => ImplementingIntrinsic<IConstraint>(sourceContext) != null;
        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) => ImplementingIntrinsic<IConstraint>(compilationContext).MatchesConstraint(value, compilationContext);
    }
}