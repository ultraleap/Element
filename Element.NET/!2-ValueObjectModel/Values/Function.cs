using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public abstract class Function : Value
    {
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) =>
            this.VerifyArgumentsAndApplyFunction(arguments, () => ResolveCall(arguments, context), context);

        protected abstract Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, Context context);

        protected IReadOnlyList<(Identifier Identifier, IValue Value)> MakeNamedArgumentList(IReadOnlyList<IValue> arguments)
        {
            if (arguments.Count < 1) return Array.Empty<(Identifier Identifier, IValue Value)>();

            var namedArguments = new List<(Identifier Identifier, IValue Value)>(InputPorts.Count);

            for (var i = 0; i < arguments.Count; i++)
            {
                var arg = arguments[i];
                var port = InputPorts[i];

                if (port?.Identifier.HasValue ?? false) // Ignore adding ports without identifier - they are discards
                {
                    namedArguments.Add((port!.Identifier!.Value, arg));
                }
            }

            return namedArguments;
        }

        public override bool IsFunction => true;

        public abstract override IReadOnlyList<ResolvedPort> InputPorts { get; }
        public abstract override IValue ReturnConstraint { get; }

        public override string SummaryString => $"({InputPortsJoined}):{ReturnConstraint.SummaryString}";
    }
    
    public class IntrinsicFunction : Function, IIntrinsicValue
    {
        IIntrinsicImplementation IIntrinsicValue.Implementation => _implementation;
        private readonly IIntrinsicFunctionImplementation _implementation;
        public override bool IsIntrinsicOfType<TIntrinsicImplementation>() => _implementation.GetType() == typeof(TIntrinsicImplementation);
        public override bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic) => _implementation == intrinsic;

        public IntrinsicFunction(IIntrinsicFunctionImplementation implementation, IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint)
        {
            _implementation = implementation;
            InputPorts = inputPorts;
            ReturnConstraint = returnConstraint;
        }

        protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, Context context) => _implementation.Call(arguments, context);
        public override IReadOnlyList<ResolvedPort> InputPorts { get; }
        public override IValue ReturnConstraint { get; }
    }

    public abstract class CustomFunction : Function
    {
        protected readonly IScope _parent;

        protected CustomFunction(IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint, IScope parent)
        {
            _parent = parent;
            InputPorts = inputPorts;
            ReturnConstraint = returnConstraint;
        }

        public override IReadOnlyList<ResolvedPort> InputPorts { get; }
        public override IValue ReturnConstraint { get; }
    }
    
    public class ExpressionBodiedFunction : CustomFunction
    {
        private readonly ExpressionBody _expressionBody;

        public ExpressionBodiedFunction(IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint, ExpressionBody expressionBody, IScope parent)
            : base(inputPorts, returnConstraint, parent) =>
            _expressionBody = expressionBody;

        protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, Context context)
        {
            context.Aspect?.BeforeExpressionBody(_expressionBody, _parent);
            var result = _expressionBody.Expression.ResolveExpression(new ResolvedBlock(MakeNamedArgumentList(arguments), _parent, () => this), context);
            return context.Aspect?.ExpressionBody(_expressionBody, _parent, result) ?? result;
        }
    }
    
    public class ScopeBodiedFunction : CustomFunction
    {
        private readonly FunctionBlock _scopeBody;

        public ScopeBodiedFunction(IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint, FunctionBlock scopeBody, IScope parent)
            : base(inputPorts, returnConstraint, parent) =>
            _scopeBody = scopeBody;

        protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, Context context)
        {
            context.Aspect?.BeforeScopeBody(_scopeBody, _parent);
            var result = _scopeBody.ResolveBlockWithCaptures(_parent, MakeNamedArgumentList(arguments), context, () => this)
                                   .Bind(localScope => localScope.Index(Parser.ReturnIdentifier, context));
            return context.Aspect?.ScopeBody(_scopeBody, _parent, result) ?? result;
        }
    }

    public static class FunctionExtensions
    {
        public static Result<IValue> VerifyArgumentsAndApplyFunction(this IValue function,
                                                                     IEnumerable<IValue> arguments,
                                                                     Func<Result<IValue>> resolveFunc,
                                                                     Context context)
        {
            if (context.CallStack.Contains(function)) return context.Trace(EleMessageCode.RecursionNotAllowed, $"Multiple references to {function} in same call stack - recursion is not allowed");
            if (context.CallStack.Count > context.CompilerOptions.CallStackLimit) return context.Trace(EleMessageCode.CallStackLimitReached, $"Call stack has exceeded limit of {context.CompilerOptions.CallStackLimit} - this can modified as a compiler option");
            context.CallStack.Push(function);

            try
            {
                Result ResultMatchesReturnConstraint(IValue result)
                {
                    return function.ReturnConstraint.MatchesConstraint(result, context)
                                   .Bind(matches => matches
                                                        ? Result.Success
                                                        : context.Trace(EleMessageCode.ConstraintNotSatisfied, $"Result '{result}' for function '{function}' does not match '{function.ReturnConstraint}' constraint"));
                }


                return CheckInputConstraints(function.InputPorts, arguments as IValue[] ?? arguments.ToArray(), context)
                       .Bind(resolveFunc)
                       .Check(ResultMatchesReturnConstraint);
            }
            finally
            {
                context.CallStack.Pop();
            }
        }
        
        private static Result CheckInputConstraints(IReadOnlyList<ResolvedPort> ports, IReadOnlyList<IValue> arguments, Context context)
        {
            if (arguments.Count <= 0) return Result.Success;
            var resultBuilder = new ResultBuilder(context);
                
            var isVariadic = ports.Any(p => p == ResolvedPort.VariadicPort);
            if (!isVariadic && arguments.Count != ports.Count)
            {
                resultBuilder.Append(EleMessageCode.ArgumentCountMismatch, $"Expected '{ports.Count}' arguments but got '{arguments.Count}'");
            }

            for (var i = 0; i < Math.Min(ports.Count, arguments.Count); i++)
            {
                var arg = arguments[i];
                var port = ports[i];
                if (port == ResolvedPort.VariadicPort)
                    break; // If we find a variadic port we can't do any more constraint checking here, it must be done within the functions implementation.
                resultBuilder.Append(port.ResolvedConstraint
                                         .MatchesConstraint(arg, context)
                                         .Bind(matches => matches
                                                            ? Result.Success
                                                            : context.Trace(EleMessageCode.ConstraintNotSatisfied, $"Value '{arg}' for port '{port}' does not match '{port.ResolvedConstraint}' constraint")));
            }

            return resultBuilder.ToResult();
        }
    }
}