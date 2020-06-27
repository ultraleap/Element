using System.Linq;

namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class CustomConstraintDeclaration : ConstraintDeclaration
    {
        protected override string IntrinsicQualifier { get; } = string.Empty;

        protected override void AdditionalValidation(ResultBuilder builder)
        {
            if (!HasDeclaredInputs)
            {
                builder.Append(MessageCode.MissingPorts, $"Non-intrinsic constraint '{Location}' must have a port list");
            }
        }

        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) =>
            value.FullyResolveValue(context)
                 .Cast<IFunctionSignature>(context)
                 .Bind(fn =>
                 {
                     // Function arity must match the constraint
                     if (fn.Inputs.Count != DeclaredInputs.Count) return false;
                     
                     var resultBuilder = new ResultBuilder<bool>(context, true);
                     
                     void CompareConstraints(Port argPort, Port declPort)
                     {
                         resultBuilder.Append(argPort.ResolveConstraint(Parent, context)
                                                     .Accumulate(() => declPort.ResolveConstraint(Parent, context))
                                                     .Do(tuple =>
                                                     {
                                                         var (argConstraint, declConstraint) = tuple;
                                                         // This port pair passes if the declarations port is Any (all constraints are narrower than Any)
                                                         // otherwise it must be exactly the same constraint since there is no type/constraint hierarchy
                                                         resultBuilder.Result &=
                                                             declConstraint.IsIntrinsicConstraint<AnyConstraint>()
                                                             || argConstraint == declConstraint;
                                                     }));
                     }

                     foreach (var (argumentPort, matchingPort) in fn.Inputs.Zip(DeclaredInputs, (argumentPort, matchingPort) => (argumentPort, matchingPort)))
                     {
                         CompareConstraints(argumentPort, matchingPort);
                     }
                     CompareConstraints(fn.Output, DeclaredOutput);

                     return resultBuilder.ToResult();
                 });
    }
}