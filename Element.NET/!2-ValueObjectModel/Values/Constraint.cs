using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public class IntrinsicConstraint : Value, IIntrinsicValue
    {
        IIntrinsicImplementation IIntrinsicValue.Implementation => _implementation;
        private readonly IIntrinsicConstraintImplementation _implementation;
        public IntrinsicConstraint(IIntrinsicConstraintImplementation implementation) => _implementation = implementation;
        public override Identifier? Identifier => _implementation.Identifier;
        public override Result<bool> MatchesConstraint(IValue value, Context context) => _implementation.MatchesConstraint(value, context);
    }
    
    public class FunctionConstraint : Value
    {
        public override Identifier? Identifier { get; }
        public override IReadOnlyList<ResolvedPort> InputPorts { get; }
        public override IValue ReturnConstraint { get; }

        public FunctionConstraint(Identifier identifier, IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint)
        {
            Identifier = identifier;
            InputPorts = inputPorts;
            ReturnConstraint = returnConstraint;
        }

        public override Result<bool> MatchesConstraint(IValue value, Context context) =>
            value.FullyResolveValue(context)
                 .Bind(fn =>
                 {
                     // Function arity must match the constraint
                     if (fn.InputPorts.Count != InputPorts.Count) return false;
                     
                     var resultBuilder = new ResultBuilder<bool>(context, true);
                     
                     void CompareConstraints(IValue argConstraint, IValue declConstraint)
                     {
                         // This port pair passes if the declarations port is Any (all constraints are narrower than Any)
                         // otherwise it must be exactly the same constraint since there is no type/constraint hierarchy
                         // ReSharper disable once PossibleUnintendedReferenceComparison
                         // TODO: Does Nothing need to be handled specially here?
                         resultBuilder.Result &=
                             declConstraint.IsIntrinsic<AnyConstraint>()
                             || argConstraint == declConstraint;
                     }

                     foreach (var (argumentPort, matchingPort) in fn.InputPorts.Zip(InputPorts, (argumentPort, matchingPort) => (argumentPort, matchingPort)))
                     {
                         CompareConstraints(argumentPort.ResolvedConstraint, matchingPort.ResolvedConstraint);
                     }
                     CompareConstraints(fn.ReturnConstraint, ReturnConstraint);

                     return resultBuilder.ToResult();
                 });
    }
}