using System.Linq;

namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class ExtrinsicConstraint : DeclaredConstraint
    {
        protected override string IntrinsicQualifier { get; } = string.Empty;

        internal override bool Validate(SourceContext sourceContext)
        {
            if (!HasDeclaredInputs)
            {
                sourceContext.LogError(13, $"Non-intrinsic constraint '{Location}' must have a port list");
                return false;
            }

            return true;
        }

        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext)
        {
            if (!(value is IFunctionSignature fn)) return false;
            if (fn.Inputs.Length != DeclaredInputs.Length) return false;
            var success = true;
            bool CompareConstraints(IConstraint argumentConstraint, IConstraint matchingConstraint)
            {
                if (matchingConstraint == AnyConstraint.Instance) return true;
                return matchingConstraint == argumentConstraint;
            }

            foreach (var (argumentPort, matchingPort) in fn.Inputs.Zip(DeclaredInputs, (argumentPort, matchingPort) => (argumentPort, matchingPort)))
            {
                success &= CompareConstraints(argumentPort.ResolveConstraint(compilationContext), matchingPort.ResolveConstraint(compilationContext));
            }

            success &= CompareConstraints(fn.Output.ResolveConstraint(compilationContext), DeclaredOutput.ResolveConstraint(compilationContext));

            return success;
        }
    }
}