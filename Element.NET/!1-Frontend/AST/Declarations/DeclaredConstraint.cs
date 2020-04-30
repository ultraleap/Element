using System.Linq;

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

        public override bool Validate(SourceContext sourceContext)
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
            if (!(value is IFunction fn)) return false;
            if (fn.Inputs.Length != DeclaredInputs.Length) return false;
            var success = true;
            bool CompareConstraints(IConstraint argumentConstraint, IConstraint matchingConstraint)
            {
                if (matchingConstraint == AnyConstraint.Instance) return true;
                return matchingConstraint == argumentConstraint;
            }

            foreach (var (argumentPort, matchingPort) in fn.Inputs.Zip(DeclaredInputs, (argumentPort, matchingPort) => (argumentPort, matchingPort)))
            {
                success &= CompareConstraints(argumentPort.ResolveConstraint(ParentScope, compilationContext), matchingPort.ResolveConstraint(ParentScope, compilationContext));
            }

            success &= CompareConstraints(fn.Output?.ResolveConstraint(ParentScope, compilationContext) ?? AnyConstraint.Instance,
                DeclaredType?.ResolveConstraint(ParentScope, compilationContext) ?? AnyConstraint.Instance);

            return success;
        }
    }

    // ReSharper disable once UnusedType.Global
    public class IntrinsicConstraint : DeclaredConstraint
    {
        protected override string IntrinsicQualifier { get; } = "intrinsic";

        public override bool Validate(SourceContext sourceContext) => ImplementingIntrinsic<IConstraint>(sourceContext) != null;
        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) => ImplementingIntrinsic<IConstraint>(null).MatchesConstraint(value, compilationContext);
    }
}