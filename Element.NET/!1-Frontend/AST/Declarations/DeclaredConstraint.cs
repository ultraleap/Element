namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class DeclaredConstraint : DeclaredItem, IConstraint
    {
        public override bool Validate(CompilationContext compilationContext)
        {
            var success = ValidateIntrinsic<IConstraint>(compilationContext);
            if (!IsIntrinsic && DeclaredInputs.Length < 1)
            {
                compilationContext.LogError(13, $"Non-intrinsic constraint '{Location}' must have a port list");
                success = false;
            }

            return success;
        }

        public override IType Type => ConstraintType.Instance;

        protected override string Qualifier { get; } = "constraint";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Terminal)};
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            IsIntrinsic
                ? ImplementingIntrinsic<IConstraint>(compilationContext)?.MatchesConstraint(value, compilationContext) ?? false
                : new[] {value}.ValidateArguments(DeclaredInputs, Parent, compilationContext);
    }
}