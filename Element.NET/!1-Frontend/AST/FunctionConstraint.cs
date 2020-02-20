namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class FunctionConstraint : CallableDeclaration<Terminal>, IConstraint
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
        public bool CanBeCached => true;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext)
        {
            if (IsIntrinsic)
            {
                if (ImplementingIntrinsic is IConstraint intrinsicConstraint)
                {
                    return intrinsicConstraint.MatchesConstraint(value, compilationContext);
                }

                if (ImplementingIntrinsic != null)
                {
                    compilationContext.LogError(14, $"Found intrinsic '{FullPath}' but it is not callable");
                    return false;
                }

                compilationContext.LogError(4, $"Intrinsic '{FullPath}' is not implemented");
                return false;
            }

            return new[] {value}.ValidateArgumentConstraints(DeclaredInputs, FindConstraint, compilationContext);
        }
    }
}