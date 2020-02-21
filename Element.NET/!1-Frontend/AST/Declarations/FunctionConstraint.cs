namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class FunctionConstraint : DeclaredCallable<Terminal, IntrinsicConstraint>, IConstraint
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
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            IsIntrinsic
                ? GetImplementingIntrinsic(compilationContext)?.MatchesConstraint(value, compilationContext) ?? false
                : new[] {value}.ValidateArgumentConstraints(DeclaredInputs, FindConstraint, compilationContext);
    }
}