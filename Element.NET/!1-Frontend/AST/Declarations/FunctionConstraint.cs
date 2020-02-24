namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class FunctionConstraint : DeclaredCallable<IntrinsicConstraint>, IConstraint
    {
        public override bool Validate(CompilationContext compilationContext)
        {
            var success = ValidateIntrinsic(compilationContext);
            if (!IsIntrinsic && DeclaredInputs.Length < 1)
            {
                compilationContext.LogError(13, $"Non-intrinsic function constraint '{FullPath}' must have a port list");
                success = false;
            }

            return success;
        }

        protected override string Qualifier { get; } = "constraint";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Terminal)};
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            IsIntrinsic
                ? GetImplementingIntrinsic(compilationContext)?.MatchesConstraint(value, compilationContext) ?? false
                : new[] {value}.ValidateArgumentConstraints(DeclaredInputs, Parent, compilationContext);
    }
}