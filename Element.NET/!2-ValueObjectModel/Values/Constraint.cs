using System.Collections.Generic;
using System.Linq;
using System.Text;
using ResultNET;

namespace Element.AST
{
    public class IntrinsicConstraint : Value, IIntrinsicValue
    {
        IIntrinsicImplementation IIntrinsicValue.Implementation => _implementation;
        private readonly IIntrinsicConstraintImplementation _implementation;
        public override bool IsIntrinsicOfType<TIntrinsicImplementation>() => _implementation is TIntrinsicImplementation;
        public override bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic) => _implementation == intrinsic;
        public IntrinsicConstraint(IIntrinsicConstraintImplementation implementation) => _implementation = implementation;
        public override Result MatchesConstraint(IValue value, Context context) => _implementation.MatchesConstraint(value, context);
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

        public override Result MatchesConstraint(IValue fn, Context context)
        {
            if (!fn.IsCallable(context)) return context.Trace(ElementMessage.ConstraintNotSatisfied, $"Expected a callable function but got {fn}");
            // Function arity must match the constraint
            if (fn.InputPorts.Count != InputPorts.Count) return context.Trace(ElementMessage.ConstraintNotSatisfied, $"Expected function with {InputPorts.Count} parameters but has {fn.InputPorts.Count}");

            StringBuilder? errorMessageBuilder = null;

            void CompareConstraintPair(IValue argConstraint, IValue expectedConstraint, bool portIsInput)
            {
                // This port pair passes if the expected port is Any (all constraints are narrower than Any)
                // otherwise it must be exactly the same constraint since there is no type/constraint hierarchy
                // TODO: Does Nothing need to be handled specially here?
                var portMatches = expectedConstraint.IsIntrinsicOfType<AnyConstraint>()
                                  || argConstraint.IsInstance(expectedConstraint);
                
                if (!portMatches)
                {
                    // On first port error, add extra message about the function constraint not matching
                    errorMessageBuilder ??= new StringBuilder($"Expected function signature to match '{this}' but was '{fn}'");
                    errorMessageBuilder.Append($"\n    Expected {(portIsInput ? "input" : "return")} port '{expectedConstraint}' but was '{argConstraint}'");
                }
            }

            foreach (var (argumentPort, matchingPort) in fn.InputPorts.Zip(InputPorts, (argumentPort, matchingPort) => (argumentPort, matchingPort)))
            {
                CompareConstraintPair(argumentPort.ResolvedConstraint, matchingPort.ResolvedConstraint, true);
            }

            CompareConstraintPair(fn.ReturnConstraint, ReturnConstraint, false);

            return errorMessageBuilder == null
                ? Result.Success
                : context.Trace(ElementMessage.ConstraintNotSatisfied, errorMessageBuilder.ToString());
        }
    }
}