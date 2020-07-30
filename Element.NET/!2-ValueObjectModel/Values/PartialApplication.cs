using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public static class PartialApplication
    {
        public static Result<IValue> PartiallyApply(this IFunctionValue function, IValue[] arguments, CompilationContext context)
        {
            var appliedFunction = function switch
            {
                AppliedFunction _ => throw new NotImplementedException("Should be implemented by make a new applied function with extra applied args (replace the applied function, don't wrap it!)"),
                _ => new AppliedFunction(arguments, function)
            };
            return appliedFunction.CanBeFullyApplied ? appliedFunction.Call(Array.Empty<IValue>(), context) : appliedFunction;
        }

        private class AppliedFunction : Function
        {
            private readonly IFunctionValue _definition;
            private readonly List<IValue> _appliedArguments;
            public AppliedFunction(IEnumerable<IValue> arguments, IFunctionValue definition)
            {
                _appliedArguments = arguments.ToList();
                _definition = definition;
            }

            public bool CanBeFullyApplied => _appliedArguments.Count >= _definition.InputPorts.Count;

            public override string SummaryString => $"{_definition} <partially applied {_appliedArguments.Count}/{_definition.InputPorts.Count}>";
            protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, CompilationContext context) => _definition.Call(_appliedArguments.Concat(arguments).ToArray(), context);
            public override IReadOnlyList<ResolvedPort> InputPorts => _definition.InputPorts.Skip(_appliedArguments.Count).ToList();
            public override IValue ReturnConstraint => _definition.ReturnConstraint;
        }
    }
}