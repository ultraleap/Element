using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public static class PartialApplication
    {
        public static Result<IValue> PartiallyApply(this IValue function, IValue[] arguments, Context context)
        {
            var appliedFunction = function switch
            {
                AppliedFunction af => new AppliedFunction(af.AppliedArguments.Concat(arguments), af.Function),
                _ => new AppliedFunction(arguments, function)
            };
            return appliedFunction.CanBeFullyApplied ? appliedFunction.Call(Array.Empty<IValue>(), context) : appliedFunction;
        }

        private class AppliedFunction : Function
        {
            public AppliedFunction(IEnumerable<IValue> arguments, IValue function)
            {
                AppliedArguments = arguments.ToList();
                Function = function;
            }

            public readonly IValue Function;
            public readonly List<IValue> AppliedArguments;
            public bool CanBeFullyApplied => AppliedArguments.Count >= Function.InputPorts.Count;

            public override Identifier? Identifier => null;
            public override string SummaryString => $"{Function} <partially applied {AppliedArguments.Count}/{Function.InputPorts.Count}>";
            protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, Context context) => Function.Call(AppliedArguments.Concat(arguments).ToArray(), context);
            public override IReadOnlyList<ResolvedPort> InputPorts => Function.InputPorts.Skip(AppliedArguments.Count).ToList();
            public override IValue ReturnConstraint => Function.ReturnConstraint;
        }
    }
}