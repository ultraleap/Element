using System.Collections.Generic;
using System.Linq;
using ResultNET;

namespace Element.AST
{
    public static class Uncurrying
    {
        public static Result<IValue> Uncurry(this IValue a, IValue b, Context context) =>
            UncurriedFunction.Create(a, b, context);

        public static Result<IValue> Uncurry(this IValue a, string bFunctionExpression, Context context, bool reverseCallOrder = false) =>
            context.EvaluateExpression(bFunctionExpression).Bind(b => reverseCallOrder ? b.Uncurry(a, context) : a.Uncurry(b, context));

        private class UncurriedFunction : Function
        {
            private readonly IValue _a;
            private readonly IValue _b;

            private UncurriedFunction(IValue a, IValue b)
            {
                _a = a;
                _b = b;
                InputPorts = _a.InputPorts.Concat(_b.InputPorts.Skip(1)).ToArray();
                ReturnConstraint = _b.ReturnConstraint;
            }
            
            public static Result<IValue> Create(IValue a, IValue b, Context context)
            {
                if (!a.HasInputs())
                {
                    return context.Trace(ElementMessage.NotFunction, $"'{a}' is not a function, only functions can be uncurried");
                }
                
                if (!b.HasInputs())
                {
                    return context.Trace(ElementMessage.NotFunction, $"'{b}' is not a function, only functions can be uncurried");
                }
                
                if (b.InputPorts.Count < 1)
                {
                    return context.Trace(ElementMessage.FunctionCannotBeUncurried, $"Function B '{b}' must have at least 1 input and where the first input must be compatible with the output of Function A");
                }

                if (a.InputPorts.Contains(ResolvedPort.VariadicPort))
                {
                    return context.Trace(ElementMessage.FunctionCannotBeUncurried, $"Function A '{a}' is variadic - variadic functions cannot be the first argument of an uncurrying operation");
                }
                
                return new UncurriedFunction(a, b);
            }

            public override IReadOnlyList<ResolvedPort> InputPorts { get; }
            public override IValue ReturnConstraint { get; }
            public override string SummaryString => $"<uncurried function {_a} << {_b}>";

            protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, Context context) =>
                _a.Call(arguments.Take(_a.InputPorts.Count).ToArray(), context)
                  // Now resolve B, skip arguments used in A
                  .Bind(resultOfA => _b.Call(arguments.Skip(_a.InputPorts.Count).Prepend(resultOfA).ToArray(), context));
        }
    }
}