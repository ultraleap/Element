using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public class IntrinsicConstraint : Value, IIntrinsicValue
    {
        IIntrinsicImplementation IIntrinsicValue.Implementation => _implementation;
        private readonly IIntrinsicConstraintImplementation _implementation;
        public IntrinsicConstraint(IIntrinsicConstraintImplementation implementation) => _implementation = implementation;
        public override Result<bool> MatchesConstraint(IValue value, Context context) => _implementation.MatchesConstraint(value, context);
    }
    
    public class FunctionConstraint : Value
    {
        public override IReadOnlyList<ResolvedPort> InputPorts { get; }
        public override IValue ReturnConstraint { get; }

        public override string SummaryString => $"({InputPortsJoined}):{ReturnConstraint.SummaryString}";

        public FunctionConstraint(IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint)
        {
            InputPorts = inputPorts;
            ReturnConstraint = returnConstraint;
        }

        public override Result<bool> MatchesConstraint(IValue fn, Context context)
        {
            // Function arity must match the constraint
            if (fn.InputPorts.Count != InputPorts.Count) return false;

            var resultBuilder = new ResultBuilder<bool>(context, true);

            void CompareConstraintPair(IValue argConstraint, IValue expectedConstraint, bool portIsInput)
            {
                var succeedingSoFar = resultBuilder.Result;
                // This port pair passes if the expected port is Any (all constraints are narrower than Any)
                // otherwise it must be exactly the same constraint since there is no type/constraint hierarchy
                // TODO: Does Nothing need to be handled specially here?
                var portMatches = expectedConstraint.IsIntrinsic<AnyConstraint>()
                                  || argConstraint == expectedConstraint;
                resultBuilder.Result &= portMatches;
                
                if (!portMatches)
                {
                    // On first port error, add extra message about the function constraint not matching
                    if (succeedingSoFar) resultBuilder.Append(EleMessageCode.ConstraintNotSatisfied, $"Expected function signature to match '{this}' but was '{fn}'");
                    resultBuilder.Append(EleMessageCode.ConstraintNotSatisfied, $"Expected {(portIsInput ? "input" : "return")} port '{expectedConstraint}' but was '{argConstraint}'");
                }
            }

            foreach (var (argumentPort, matchingPort) in fn.InputPorts.Zip(InputPorts, (argumentPort, matchingPort) => (argumentPort, matchingPort)))
            {
                CompareConstraintPair(argumentPort.ResolvedConstraint, matchingPort.ResolvedConstraint, true);
            }

            CompareConstraintPair(fn.ReturnConstraint, ReturnConstraint, false);

            return resultBuilder.ToResult();
        }
    }
}