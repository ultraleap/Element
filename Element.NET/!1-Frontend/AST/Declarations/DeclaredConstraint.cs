namespace Element.AST
{
    public abstract class DeclaredConstraint : Declaration, IConstraint
    {
        protected override string Qualifier { get; } = "constraint";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Terminal)};
        public override IType Type => ConstraintType.Instance;
        public abstract bool MatchesConstraint(IValue value, CompilationContext compilationContext);
    }

    // ReSharper disable once UnusedType.Global
    public class ExtrinsicConstraint : DeclaredConstraint
    {
        protected override string IntrinsicQualifier { get; } = string.Empty;
        public override bool Validate(CompilationContext compilationContext)
        {
            if (DeclaredInputs.Length < 1)
            {
                compilationContext.LogError(13, $"Non-intrinsic constraint '{Location}' must have a port list");
                return false;
            }

            return true;
        }
        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            new[] {value}.ValidateArguments(DeclaredInputs, ParentScope, compilationContext);
    }

    // ReSharper disable once UnusedType.Global
    public class IntrinsicConstraint : DeclaredConstraint
    {
        protected override string IntrinsicQualifier { get; } = "intrinsic";
        public override bool Validate(CompilationContext compilationContext) =>
            ImplementingIntrinsic<IConstraint>(compilationContext) != null;
        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            ImplementingIntrinsic<IConstraint>(compilationContext)?.MatchesConstraint(value, compilationContext) ??
            false;
    }
}