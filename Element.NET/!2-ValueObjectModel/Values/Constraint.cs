using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public class IntrinsicConstraint : Value, IIntrinsicValue
    {
        IIntrinsicImplementation IIntrinsicValue.Implementation => _implementation;
        private readonly IIntrinsicConstraintImplementation _implementation;
        public IntrinsicConstraint(IIntrinsicConstraintImplementation implementation, string? location = null) :base(location) => _implementation = implementation;
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => _implementation.MatchesConstraint(value, context);
    }
    
    public class FunctionConstraint : Value, IFunctionSignature
    {
        public IReadOnlyList<ResolvedPort> InputPorts { get; }
        public IValue ReturnConstraint { get; }

        public FunctionConstraint(IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint, string? location = null) :base(location)
        {
            InputPorts = inputPorts;
            ReturnConstraint = returnConstraint;
        }
        
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) =>
            value.FullyResolveValue(context)
                 .Cast<IFunctionSignature>(context)
                 .Bind(fn =>
                 {
                     // Function arity must match the constraint
                     if (fn.InputPorts.Count != InputPorts.Count) return false;
                     
                     var resultBuilder = new ResultBuilder<bool>(context, true);
                     
                     void CompareConstraints(IValue argConstraint, IValue declConstraint)
                     {
                         // This port pair passes if the declarations port is Any (all constraints are narrower than Any)
                         // otherwise it must be exactly the same constraint since there is no type/constraint hierarchy
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