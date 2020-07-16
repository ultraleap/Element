using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public static class Uncurrying
    {
        public static Result<IValue> Uncurry(this IFunctionValue a, IFunctionValue b, ITrace trace) =>
            UncurriedFunction.Create(a, b, trace);

        public static Result<IValue> Uncurry(this IFunctionValue a, string bFunctionExpression,
                                                  SourceContext context) =>
            context.EvaluateExpression(bFunctionExpression).Cast<IFunctionValue>(context).Bind(fn => a.Uncurry(fn, context));

        private class UncurriedFunction : Function
        {
            private readonly IFunctionValue _a;
            private readonly IFunctionValue _b;

            private UncurriedFunction(IFunctionValue a, IFunctionValue b)
            {
                _a = a;
                _b = b;
                InputPorts = _a.InputPorts.Concat(_b.InputPorts.Skip(1)).ToArray();
                ReturnConstraint = _b.ReturnConstraint;
            }
            
            public static Result<IValue> Create(IFunctionValue a, IFunctionValue b, ITrace trace)
            {
                if (b.InputPorts.Count < 1)
                {
                    return trace.Trace(MessageCode.FunctionCannotBeUncurried, $"Function B '{b}' must have at least 1 input and where the first input must be compatible with the output of Function A");
                }

                // TODO: Disallow variadics as A
                if (a.InputPorts.Contains(ResolvedPort.VariadicPort))
                {
                    return trace.Trace(MessageCode.FunctionCannotBeUncurried, $"Function A '{a}' is variadic - variadic functions cannot be the first argument of an uncurrying operation");
                }
                
                return new UncurriedFunction(a, b);
            }

            public override IReadOnlyList<ResolvedPort> InputPorts { get; }
            public override IValue ReturnConstraint { get; }
            protected override string ToStringInternal() => $"({_a} << {_b}):Function";

            protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, CompilationContext context) =>
                _a.Call(arguments.Take(_a.InputPorts.Count).ToArray(), context)
                  // Now resolve B, skip arguments used in A
                  .Bind(resultOfA => _b.Call(arguments.Skip(_a.InputPorts.Count).Prepend(resultOfA).ToArray(), context));
        }
    }
}