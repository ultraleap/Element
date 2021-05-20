using System;
using System.Collections.Generic;
using System.Linq;
using ResultNET;

namespace Element.AST
{
    public static class PartialApplication
    {
        public static Result<IValue> PartiallyApply(this IValue function, IValue[] arguments, Context context)
        {
            if (!function.HasInputs()) return context.Trace(ElementMessage.NotFunction, "Cannot partially apply arguments to a non-function value");
            var appliedFunction = function.InnerIs(out AppliedFunction af)
                                      ? new AppliedFunction(af.AppliedArguments.Concat(arguments), af.WrappedValue)
                                      : new AppliedFunction(arguments, function);
            
            return appliedFunction.CanBeFullyApplied
                       ? appliedFunction.Call(Array.Empty<IValue>(), context)
                       : appliedFunction;
        }

        private class AppliedFunction : WrapperValue
        {
            public AppliedFunction(IEnumerable<IValue> arguments, IValue function)
                : base(function) =>
                AppliedArguments = arguments.ToList();

            public override string TypeOf => "AppliedFunction";

            public readonly List<IValue> AppliedArguments;
            public bool CanBeFullyApplied => AppliedArguments.Count >= WrappedValue.InputPorts.Count;
            public override string SummaryString => $"{WrappedValue} <partially applied {AppliedArguments.Count}/{WrappedValue.InputPorts.Count}>";
            public override Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) => this.VerifyArgumentsAndApplyFunction(arguments, () => WrappedValue.Call(AppliedArguments.Concat(arguments).ToArray(), context), context);
            public override IReadOnlyList<ResolvedPort> InputPorts => WrappedValue.InputPorts.Skip(AppliedArguments.Count).ToList();
        }
    }
}