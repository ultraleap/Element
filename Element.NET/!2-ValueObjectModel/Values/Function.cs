using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public abstract class Function : Value, IFunctionValue
    {
        protected Function(string? location = null) : base(location) { }

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
            this.VerifyArgumentsAndApplyFunction(arguments, () => ResolveFunctionBody(arguments, context), context);

        protected abstract Result<IValue> ResolveFunctionBody(IReadOnlyList<IValue> arguments, CompilationContext context);
        
        protected IScope MakeArgumentScope(IReadOnlyList<IValue> arguments, IScope? parent)
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

            return new Scope(namedArguments, parent);
        }

        public abstract IReadOnlyList<ResolvedPort> InputPorts { get; }
        public abstract IValue ReturnConstraint { get; }
    }
    
    public class ExpressionBodiedFunction : Function
    {
        private readonly ExpressionBody _expressionBody;
        private readonly IScope _parent;

        public ExpressionBodiedFunction(IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint, ExpressionBody expressionBody, IScope parent, string? location = null)
            : base(location)
        {
            _expressionBody = expressionBody;
            _parent = parent;
            InputPorts = inputPorts;
            ReturnConstraint = returnConstraint;
        }
        
        protected override Result<IValue> ResolveFunctionBody(IReadOnlyList<IValue> arguments, CompilationContext context) =>
            _expressionBody.Expression.ResolveExpression(MakeArgumentScope(arguments, _parent), context);

        public override IReadOnlyList<ResolvedPort> InputPorts { get; }
        public override IValue ReturnConstraint { get; }
    }
    
    public class ScopeBodiedFunction : Function
    {
        private readonly FunctionBlock _scopeBody;
        private readonly IScope _parent;

        public ScopeBodiedFunction(IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint, FunctionBlock scopeBody, IScope parent, string? location = null)
            : base(location)
        {
            InputPorts = inputPorts;
            ReturnConstraint = returnConstraint;
            _scopeBody = scopeBody;
            _parent = parent;
        }

        protected override Result<IValue> ResolveFunctionBody(IReadOnlyList<IValue> arguments, CompilationContext context) =>
            _scopeBody.Resolve(null, context)
                      .Bind(scopeBody => MakeArgumentScope(arguments, null).CombineScopes(scopeBody, _parent, context))
                      .Bind(localScope => localScope.Index(Parser.ReturnIdentifier, context));

        public override IReadOnlyList<ResolvedPort> InputPorts { get; }
        public override IValue ReturnConstraint { get; }
    }
    
    public class IntrinsicFunction : Function
    {
        public IntrinsicFunctionImplementation Implementation { get; }

        public IntrinsicFunction(IntrinsicFunctionImplementation implementation, IReadOnlyList<ResolvedPort> inputPorts, IValue returnConstraint, string? location = null)
            : base(location)
        {
            Implementation = implementation;
            InputPorts = inputPorts;
            ReturnConstraint = returnConstraint;
        }

        protected override Result<IValue> ResolveFunctionBody(IReadOnlyList<IValue> arguments, CompilationContext context) => Implementation.Call(arguments, context);
        public override IReadOnlyList<ResolvedPort> InputPorts { get; }
        public override IValue ReturnConstraint { get; }
    }

    public static class FunctionExtensions
    {
        public static bool IsNullary(this IFunctionSignature functionSignature) => functionSignature.InputPorts.Count == 0;
        
        public static Result<IValue> VerifyArgumentsAndApplyFunction(this IFunctionValue function,
                                                                     IEnumerable<IValue> arguments,
                                                                     Func<Result<IValue>> resolveFunc,
                                                                     CompilationContext context)
        {
            if (context.ContainsFunction(function)) return context.Trace(MessageCode.RecursionNotAllowed, $"Multiple references to {function} in same call stack - recursion is not allowed");
            context.PushFunction(function);

            try
            {
                Result ResultMatchesReturnConstraint(IValue result) =>
                    function.ReturnConstraint.MatchesConstraint(result, context)
                            .Bind(matches => matches
                                                 ? Result.Success
                                                 : context.Trace(MessageCode.ConstraintNotSatisfied, $"Result '{result}' for function '{function}' does not match '{function.ReturnConstraint}' constraint"));

                Result<IValue> FullyResolveResult(IValue returnValue) => returnValue.FullyResolveValue(context);

                return CheckInputConstraints(function.InputPorts, arguments as IValue[] ?? arguments.ToArray(), context)
                       .Bind(resolveFunc)
                       .Bind(FullyResolveResult)
                       .Check(ResultMatchesReturnConstraint);
            }
            finally
            {
                context.PopFunction();
            }
        }
        
        private static Result CheckInputConstraints(IReadOnlyList<ResolvedPort> ports, IReadOnlyList<IValue> arguments, CompilationContext context)
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