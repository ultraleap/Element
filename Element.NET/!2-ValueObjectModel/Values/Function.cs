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

        public abstract override IReadOnlyList<ResolvedPort> InputPorts { get; }
        public abstract override IValue ReturnConstraint { get; }
    }
    
    public class IntrinsicFunction : Function, IIntrinsicValue
    {
        IIntrinsicImplementation IIntrinsicValue.Implementation => _implementation;
        private readonly IIntrinsicFunctionImplementation _implementation;

        public IntrinsicFunction(IIntrinsicFunctionImplementation implementation, IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint)
        {
            _implementation = implementation;
            InputPorts = inputPorts;
            ReturnConstraint = returnConstraint;
        }

        protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, Context context) => _implementation.Call(arguments, context);
        public override Identifier? Identifier => _implementation.Identifier;
        public override IReadOnlyList<ResolvedPort> InputPorts { get; }
        public override IValue ReturnConstraint { get; }
    }

    public abstract class CustomFunction : Function
    {
        protected readonly IScope _parent;

        protected CustomFunction(Identifier? identifier, IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint, IScope parent)
        {
            Identifier = identifier;
            _parent = parent;
            InputPorts = inputPorts;
            ReturnConstraint = returnConstraint;
        }

        public override Identifier? Identifier { get; }
        public override IReadOnlyList<ResolvedPort> InputPorts { get; }
        public override IValue ReturnConstraint { get; }
    }
    
    public class ExpressionBodiedFunction : CustomFunction
    {
        private readonly ExpressionBody _expressionBody;

        public ExpressionBodiedFunction(Identifier? identifier, IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint, ExpressionBody expressionBody, IScope parent)
            : base(identifier, inputPorts, returnConstraint, parent) =>
            _expressionBody = expressionBody;

        protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, Context context) =>
            _expressionBody.Expression.ResolveExpression(new ResolvedBlock(null, MakeNamedArgumentList(arguments), _parent), context);
    }
    
    public class ScopeBodiedFunction : CustomFunction
    {
        private readonly FunctionBlock _scopeBody;

        public ScopeBodiedFunction(Identifier? identifier, IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint, FunctionBlock scopeBody, IScope parent)
            : base(identifier, inputPorts, returnConstraint, parent) =>
            _scopeBody = scopeBody;

        protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, Context context) =>
            _scopeBody.ResolveBlockWithCaptures(_parent, MakeNamedArgumentList(arguments), context)
                      .Bind(localScope => localScope.Index(Parser.ReturnIdentifier, context));
    }

    public static class FunctionExtensions
    {
        
        public static Result<IValue> VerifyArgumentsAndApplyFunction(this IValue function,
                                                                     IEnumerable<IValue> arguments,
                                                                     Func<Result<IValue>> resolveFunc,
                                                                     Context context)
        {
            if (context.CallStack.Contains(function)) return context.Trace(MessageCode.RecursionNotAllowed, $"Multiple references to {function} in same call stack - recursion is not allowed");
            context.CallStack.Push(function);

            try
            {
                Result ResultMatchesReturnConstraint(IValue result) =>
                    function.ReturnConstraint.MatchesConstraint(result, context)
                            .Bind(matches => matches
                                                 ? Result.Success
                                                 : context.Trace(MessageCode.ConstraintNotSatisfied, $"Result '{result}' for function '{function}' does not match '{function.ReturnConstraint}' constraint"));


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
                resultBuilder.Append(MessageCode.ArgumentCountMismatch, $"Expected '{ports.Count}' arguments but got '{arguments.Count}'");
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
                                                            : context.Trace(MessageCode.ConstraintNotSatisfied, $"Value '{arg}' for port '{port}' does not match '{port.ResolvedConstraint}' constraint")));
            }

            return resultBuilder.ToResult();
        }
    }
}