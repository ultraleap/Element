using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IInstancable<out TDefinition> where TDefinition : IValue
    {
        TDefinition GetDefinition(CompilationContext compilationContext);
    }
    
    public interface IFunction : IValue, IInstancable<IFunction>
    {
        Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context);
        Port[] Inputs { get; }
        Port Output { get; }
    }

    public static class FunctionHelpers
    {
        public static bool IsNullary(this IFunction Function) => Function.Inputs.Length == 0;

        public static Result<IValue> ApplyFunction(IFunction function, IReadOnlyList<IValue> arguments,
                                                   IScope callScope, bool allowPartialApplication, CompilationContext context)
        {
            var definition = function.GetDefinition(context);
            if (context.ContainsFunction(definition) && !(function is AppliedFunctionBase)) return context.Trace(MessageCode.CircularCompilation, $"Multiple references to {function} in same call stack - Recursion is disallowed");
            context.PushFunction(definition);
            var shownInTrace = false;
            if (function is IDeclared declared)
            {
                shownInTrace = true;
                context.PushTrace(declared.MakeTraceSite(function.ToString()));
            }

            try
            {
                var callScope = function switch
                {
                    // If the signature is an applied function we want to use it as the call scope. More arguments are filled by subsequent applications until full application.
                    AppliedFunctionBase appliedFunction => appliedFunction,
                    // If the signature is a function with a body then we use the bodies parent
                    IFunctionWithBody functionWithBody when functionWithBody.Body is Scope scope => scope.Declarer.Parent,
                    Declaration declaration => declaration.Parent,
                    _ => context.SourceContext.GlobalScope
                };

                
                // Fully resolve arguments
                return arguments.Select(arg => arg.FullyResolveValue(context)).BindEnumerable(ResolveCall);
                
                
                
                Result<IValue> ResolveCall(IEnumerable<IValue> args)
                {
                    arguments = args.ToArray();
                    return CheckArguments(function, arguments, callScope, allowPartialApplication, context)
                        .Bind(() =>
                                  (function switch
                                      {
                                          // When there's no arguments we can just resolve immediately
                                          IFunctionWithBody functionWithBody when arguments.Length > 0 => new AppliedFunctionWithBody(arguments, functionWithBody, callScope),
                                          IFunctionWithBody functionWithBody => functionWithBody.ResolveReturn(callScope, ResolveFunctionBody(functionWithBody.Body, callScope, context), context),
                                          { } fn when arguments.Length > 0 => new AppliedFunction(arguments, fn, callScope),
                                          { } fn => function.ResolveReturn(callScope, fn.Call(arguments, context), context),
                                          _ => throw new InternalCompilerException($"{function} function type not resolvable")
                                      }).Bind(returnValue => returnValue.FullyResolveValue(context)));
                }
            }
            finally
            {
                context.PopFunction();
                if (shownInTrace)
                {
                    context.PopTrace();
                }
            }
        }

        private static Result CheckArguments(IFunction function, IValue[] arguments, IScope callScope, bool allowPartialApplication, CompilationContext context)
        {
            if (arguments.Length <= 0) return Result.Success;
            var resultBuilder = new ResultBuilder(context);
                
            var ports = function.Inputs;
            var isVariadic = ports.Any(p => p == Port.VariadicPort);
            if (!allowPartialApplication && !isVariadic && arguments.Length != ports.Length
                || !allowPartialApplication && arguments.Length < ports.Length)
            {
                resultBuilder.Append(MessageCode.ArgumentCountMismatch, $"Expected '{ports.Length}' arguments but got '{arguments.Length}'");
            }

            for (var i = 0; i < Math.Min(ports.Length, arguments.Length); i++)
            {
                var arg = arguments[i];
                var port = ports[i];
                if (port == Port.VariadicPort)
                    break; // If we find a variadic port we can't do any more constraint checking here, it must be done within the functions implementation.
                resultBuilder.Append(port.ResolveConstraint(callScope, context)
                                         .Do(constraint => constraint.MatchesConstraint(arg, context)
                                                                     .Do(matches => matches
                                                                                        ? Result.Success
                                                                                        : context.Trace(MessageCode.ConstraintNotSatisfied, $"Value '{arg}' given for port '{port}' does not match '{constraint}' constraint"))));
            }

            return resultBuilder.ToResult();
        }

        private static Result<IValue> ResolveReturn(this IFunction function, IScope callScope, IValue result, CompilationContext context) =>
            function.Output.ResolveConstraint(callScope, context)
                    .Bind(constraint => result.FullyResolveValue(context)
                                              .Bind(fullyResolvedResult => constraint.MatchesConstraint(fullyResolvedResult, context)
                                                                                     .Bind(matches => matches
                                                                                                          ? new Result<IValue>(fullyResolvedResult)
                                                                                                          : context.Trace(MessageCode.ConstraintNotSatisfied, $"Result '{fullyResolvedResult}' for function '{function.GetDefinition(context)}' does not match '{constraint}' constraint"))));

        private abstract class AppliedFunctionBase : ScopeBase, IFunction
        {
            protected AppliedFunctionBase(IValue[] arguments, IFunction definition, IScope parent)
            {
                _definition = definition;
                _parent = parent;
                _inputs = definition.Inputs.Skip(arguments.Length).ToArray();
                _source = arguments.WithoutDiscardedArguments(definition.Inputs).ToList();
            }

            private readonly IScope _parent;
            private readonly Port[] _inputs;
            private readonly IFunction _definition;

            Port[] IFunction.Inputs => _inputs;
            Port IFunction.Output => _definition.Output;
            public abstract Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context);

            public override string ToString() => _definition.ToString();

            protected virtual IScope? _child => null;
            protected override IList<(Identifier Identifier, IValue Value)> _source { get; }

            public override Result<IValue> this[Identifier id, bool recurse, CompilationContext context] =>
                _child?[id, false, context]
                    .Else(() => Index(id))
                    .ElseIf(recurse, () => _parent[id, true, context])
                ?? Index(id)
                    .ElseIf(recurse, () => _parent[id, true, context]);

            public IFunction GetDefinition(CompilationContext compilationContext) => _definition;
        }

        private class AppliedFunction : AppliedFunctionBase
        {
            public AppliedFunction(IValue[] arguments, IFunction definition, IScope parent)
                : base(arguments, definition, parent) =>
                _definition = definition;

            private readonly IFunction _definition;

            public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
                _definition.Call(this.Concat(arguments).ToArray(), context)
                           .Bind(result => this.ResolveReturn(this, result, context));
        }

        private class AppliedFunctionWithBody : AppliedFunctionBase
        {
            public AppliedFunctionWithBody(IValue[] arguments, IFunction functionDefinition, object functionBody, IScope parent)
                : base(arguments, functionDefinition, parent)
            {
                Body = functionBody switch
                {
                    ExpressionBody exprBody => exprBody,
                    IScope scopeBody => scopeBody switch
                    {
                        IDeclared declared => scopeBody.Clone(declared.Declarer, this),
                        _ => throw new InternalCompilerException("Cannot clone non-declared scope")
                    },
                    _ => throw new InternalCompilerException("Cannot create function instance as function body type is not recognized")
                };
            }

            protected override IScope? _child => Body as IScope;

            public object Body { get; }
        }

        public static Result<IFunction> Uncurry(this IFunction a, IFunction b, ITrace trace) =>
            UncurriedFunction.Create(a, b, trace);

        public static Result<IFunction> Uncurry(this IFunction a, string bFunctionExpression,
                                                  SourceContext context) =>
            context.EvaluateExpressionAs<IFunction>(bFunctionExpression).Bind(a.Uncurry);

        private class UncurriedFunction : IFunction
        {
            private readonly IFunction _a;
            private readonly IFunction _b;

            private UncurriedFunction(IFunction a, IFunction b)
            {
                _a = a;
                _b = b;
                Inputs = _a.Inputs.Concat(_b.Inputs.Skip(1)).ToArray();
                Output = _b.Output;
            }
            
            public static Result<IFunction> Create(IFunction a, IFunction b, ITrace trace)
            {
                if (b.Inputs.Length < 1)
                {
                    return trace.Trace(MessageCode.FunctionCannotBeUncurried, $"Function B '{b}' must have at least 1 input and where the first input must be compatible with the output of Function A");
                }

                if (a.Inputs.Any(p => p == Port.VariadicPort))
                {
                    return trace.Trace(MessageCode.FunctionCannotBeUncurried, $"Function A '{a}' is variadic - variadic functions cannot be the first argument of an uncurrying operation");
                }

                if (a.Inputs.Concat(b.Inputs).Any(p => p?.Identifier == null))
                {
                    return trace.Trace(MessageCode.FunctionCannotBeUncurried, "Cannot uncurry functions with discarded/unnamed ports");
                }

                foreach (var aPort in a.Inputs)
                {
                    // TODO: Better solution to this error, it's very annoying to not be allowed to uncurry functions just because of port names!
                    Identifier? id = null;
                    if (b.Inputs.Skip(1).Any(bPort => aPort.Identifier.Equals(id = bPort.Identifier)))
                    {
                        return trace.Trace(MessageCode.FunctionCannotBeUncurried, $"'{id}' is defined on both functions, these functions cannot be uncurried");
                    }
                }
                
                return new UncurriedFunction(a, b);
            }

            IFunction IInstancable<IFunction>.GetDefinition(CompilationContext compilationContext) => this;
            public Port[] Inputs { get; }
            public Port Output { get; }
            public Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
                _a.Call(arguments.Take(_a.Inputs.Length).ToArray(), context)
                  // Now resolve B, skip arguments used in A
                  .Bind(resultOfA => _b.Call(arguments.Skip(_a.Inputs.Length).Prepend(resultOfA).ToArray(), context));
        }
    }
}