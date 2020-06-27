using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IValue
    {
        string ToString();
        //string NormalFormString { get; }
        Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context);
        Result<IValue> Index(Identifier id, CompilationContext context);
        IReadOnlyList<IValue> Members { get; }
        Result<bool> MatchesConstraint(IValue value, CompilationContext context);
        Result<IValue> DefaultValue(CompilationContext context);
        void Serialize(ResultBuilder<List<Element.Expression>> resultBuilder);
        Result<IValue> Deserialize(Func<Element.Expression> nextValue, ITrace trace);
    }
    
    public interface IFunctionSignature
    {
        Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context);
        IReadOnlyList<Port> Inputs { get; }
        Port Output { get; }
    }

    public abstract class Value : IValue
    {
        public abstract override string ToString();
        //public abstract string NormalFormString { get; }
        public virtual Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) => context.Trace(MessageCode.InvalidExpression, $"'{this}' cannot be called, it is not a function");
        public virtual Result<IValue> Index(Identifier id, CompilationContext context) => context.Trace(MessageCode.InvalidExpression, $"'{this}' is not indexable");
        public virtual IReadOnlyList<IValue> Members => Array.Empty<IValue>();
        public virtual Result<bool> MatchesConstraint(IValue value, CompilationContext context) => context.Trace(MessageCode.InvalidExpression, $"'{this}' cannot be used as a port annotation, it is not a constraint");
        public virtual Result<IValue> DefaultValue(CompilationContext context) => context.Trace(MessageCode.ConstraintNotSatisfied, $"'{this}' cannot produce a default value, only serializable types can produce default values");
        public virtual void Serialize(ResultBuilder<List<Element.Expression>> resultBuilder) => resultBuilder.Append(MessageCode.SerializationError, $"'{this}' is not serializable");
        public virtual Result<IValue> Deserialize(Func<Element.Expression> nextValue, ITrace trace) => trace.Trace(MessageCode.SerializationError, $"'{this}' cannot be deserialized");
    }

    public static class ValueExtensions
    {
        public static Result<IValue> FullyResolveValue(this IValue value, CompilationContext context) =>
            (value is IFunctionSignature fn && fn.IsNullary()
                 ? value.Call(Array.Empty<IValue>(), context)
                 : new Result<IValue>(value))
            .Map(v => v is Element.Expression expr ? expr.FoldConstants() : v)
            // ReSharper disable once PossibleUnintendedReferenceComparison
            .Bind(v => v != value ? v.FullyResolveValue(context) : new Result<IValue>(v)); // Recurse until the resolved value is the same

        public static IEnumerable<(Identifier Identifier, IValue Value)> WithoutDiscardedArguments(this IEnumerable<IValue> arguments, IEnumerable<Port> ports)
        {
            var argArray = arguments.ToArray();
            var portArray = ports.ToArray();

            var result = new List<(Identifier Identifier, IValue Value)>(argArray.Length);
            var variadicArgNumber = 0;

            // Keeps iterating until we've checked all arguments that we can
            // There can be more arguments than ports if the function is variadic
            for (var i = 0; i < argArray.Length; i++)
            {
                var arg = argArray[i];
                var port = i < portArray.Length ? portArray[i] : null;
                // port can be null if we are checking a variadic function
                if (port == Port.VariadicPort || variadicArgNumber > 0)
                {
                    result.Add((new Identifier($"varg{variadicArgNumber}"), arg));
                    variadicArgNumber++;
                }
                else if ((port?.Identifier.HasValue ?? false) && arg != null)
                {
                    result.Add((port!.Identifier!.Value, arg));
                }
            }

            return result;
        }

        public static Result<List<Element.Expression>> Serialize(this IValue value, ITrace trace)
        {
            var result = new ResultBuilder<List<Element.Expression>>(trace, new List<Element.Expression>());
            value.Serialize(result);
            return result.ToResult();
        }

        public static bool IsSerializable(this IValue value, ITrace trace) => value.Serialize(trace).IsSuccess;

        public static bool IsIntrinsicConstraint<TIntrinsicConstraint>(this IValue constraint)
            where TIntrinsicConstraint : IntrinsicConstraint =>
            constraint is IntrinsicConstraintDeclaration declaration && declaration.ImplementingIntrinsic.GetType() == typeof(TIntrinsicConstraint);
        
        public static bool IsIntrinsicType<TIntrinsicType>(this IValue type)
            where TIntrinsicType : IntrinsicType =>
            type is IntrinsicStructDeclaration declaration && declaration.ImplementingIntrinsic.GetType() == typeof(TIntrinsicType);
        
        public static bool IsIntrinsicType(this IValue type, IntrinsicType intrinsicType) =>
            type is IntrinsicStructDeclaration declaration && declaration.ImplementingIntrinsic == intrinsicType;

        public static Result<int> SerializedSize(this IValue value, ITrace trace)
        {
            var size = 0;
            return value.Deserialize(() =>
            {
                size++;
                return Constant.Zero;
            }, trace).Map(_ => size); // Discard the value and just check the size
        }

        public static Result<IValue> Deserialize(this IValue value, IEnumerable<Element.Expression> expressions, ITrace trace) =>
            value.Deserialize(new Queue<Element.Expression>(expressions).Dequeue, trace);

        public static Result<float[]> ToFloatArray(this IEnumerable<Element.Expression> expressions, ITrace trace)
        {
            var exprs = expressions as Element.Expression[] ?? expressions.ToArray();
            var result = new float[exprs.Length];
            for (var i = 0; i < result.Length; i++)
            {
                var expr = exprs[i];
                var val = (expr as Constant)?.Value;
                if (val.HasValue)
                {
                    result[i] = val!.Value;
                }
                else
                {
                    return trace.Trace(MessageCode.SerializationError, $"Non-constant expression '{expr}' cannot be evaluated to a float");
                }
            }

            return result;
        }
        
        /// <summary>
        /// Enumerates all values in the given scope returning those matching the given filter.
        /// </summary>
        public static List<IValue> EnumerateRecursively(this IScope scope, Predicate<IValue> filter = null)
        {
            IEnumerable<IValue> Recurse(IValue value) => value.Members.Where(m => filter?.Invoke(m) ?? true).SelectMany(Recurse);
            return scope.Members.SelectMany(Recurse).ToList();
        }

        public static List<Declaration> EnumerateDeclarationsRecursively(this IScope scope, Predicate<Declaration> filter = null) =>
            EnumerateRecursively(scope, value => value is IDeclared)
                .Select(v => ((IDeclared) v).Declarer)
                .Where(d => filter?.Invoke(d) ?? true)
                .ToList();
    }

    public static class FunctionHelpers
    {
        public static bool IsNullary(this IFunctionSignature functionSignature) => functionSignature.Inputs.Count == 0;

        public static Result<IValue> PartiallyApply(this IFunctionSignature functionSignature, IValue[] arguments, CompilationContext context)
        {
            /*{
                // When there's no arguments we can just resolve immediately
                IFunctionWithBody functionWithBody when arguments.Length > 0 => new AppliedFunctionWithBody(arguments, functionWithBody, callScope),
                IFunctionWithBody functionWithBody => functionWithBody.ResolveReturn(callScope, ResolveFunctionBody(functionWithBody.Body, callScope, context), context),
                { } fn when arguments.Length > 0 => new AppliedFunction(arguments, fn, callScope),
                { } fn => function.CheckOutputConstraint(callScope, fn.Call(arguments, context), context),
                _ => throw new InternalCompilerException($"{function} function type not resolvable")
            }*/
        }

        public static Result<IValue> ApplyFunction(IValue callable,
                                                   IFunctionSignature functionSignature,
                                                   IEnumerable<IValue> arguments,
                                                   IScope callScope,
                                                   Func<Result<IValue>> resolveFunc,
                                                   bool allowPartialApplication,
                                                   CompilationContext context)
        {
            if (context.ContainsFunction(callable)) return context.Trace(MessageCode.CircularCompilation, $"Multiple references to {callable} in same call stack - Recursion is disallowed");
            context.PushFunction(callable);

            var shownInTrace = false;
            if (callable is IDeclared declared)
            {
                shownInTrace = true;
                context.PushTrace(declared.MakeTraceSite(callable.ToString()));
            }

            try
            {
                // TODO: Make and push capture scope with arguments and functions locals
                return CheckInputConstraints(functionSignature, arguments as IValue[] ?? arguments.ToArray(), callScope, allowPartialApplication, context)
                       .Bind(resolveFunc)
                       .Bind(returnValue => returnValue.FullyResolveValue(context));
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
    

    public static Result<IValue> ResolveFunctionBody(object body, IScope scope, CompilationContext compilationContext) =>
            body switch
            {
                // If a function has expression body we just compile the single expression using the call scope
                ExpressionBody exprBody => exprBody.Expression.ResolveExpression(scope, compilationContext),
                // If a function has a scope body we need to find the return identifier
                IScope scopeBody => scopeBody.Index(Parser.ReturnIdentifier, compilationContext),
                _ => throw new InternalCompilerException("Function body type unrecognized")
            };

        private static Result CheckInputConstraints(IFunctionSignature functionSignature, IReadOnlyList<IValue> arguments, IScope callScope, bool allowPartialApplication, CompilationContext context)
        {
            if (arguments.Count <= 0) return Result.Success;
            var resultBuilder = new ResultBuilder(context);
                
            var ports = functionSignature.Inputs;
            var isVariadic = ports.Any(p => p == Port.VariadicPort);
            if (!allowPartialApplication && !isVariadic && arguments.Count != ports.Count
                || !allowPartialApplication && arguments.Count < ports.Count)
            {
                resultBuilder.Append(MessageCode.ArgumentCountMismatch, $"Expected '{ports.Count}' arguments but got '{arguments.Count}'");
            }

            for (var i = 0; i < Math.Min(ports.Count, arguments.Count); i++)
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

        private static Result<IValue> CheckOutputConstraint(this IFunctionSignature functionSignature, IScope callScope, IValue result, CompilationContext context) =>
            functionSignature.Output.ResolveConstraint(callScope, context)
                    .Accumulate(() => result.FullyResolveValue(context))
                    .Bind(tuple =>
                    {
                        var (constraint, fullyResolvedResult) = tuple;
                        return constraint.MatchesConstraint(fullyResolvedResult, context)
                                         .Bind(matches => matches
                                                              ? new Result<IValue>(fullyResolvedResult)
                                                              : context.Trace(MessageCode.ConstraintNotSatisfied, $"Result '{fullyResolvedResult}' for function '{functionSignature}' does not match '{constraint}' constraint"));
                    });

        /*private abstract class AppliedFunctionBase : ScopeBase, IFunction
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

            IReadOnlyList<Port> IFunction.Inputs => _inputs;
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
                           .Bind(result => this.CheckOutputConstraint(this, result, context));
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
        }*/

        public static Result<IValue> Uncurry(this IFunctionSignature a, IFunctionSignature b, ITrace trace) =>
            UncurriedFunctionSignature.Create(a, b, trace);

        public static Result<IValue> Uncurry(this IFunctionSignature a, string bFunctionExpression,
                                                  SourceContext context) =>
            context.EvaluateExpression(bFunctionExpression).Cast<IFunctionSignature>(context).Bind(fn => a.Uncurry(fn, context));

        private class UncurriedFunctionSignature : Value, IFunctionSignature
        {
            private readonly IFunctionSignature _a;
            private readonly IFunctionSignature _b;

            private UncurriedFunctionSignature(IFunctionSignature a, IFunctionSignature b)
            {
                _a = a;
                _b = b;
                Inputs = _a.Inputs.Concat(_b.Inputs.Skip(1)).ToArray();
                Output = _b.Output;
            }
            
            public static Result<IValue> Create(IFunctionSignature a, IFunctionSignature b, ITrace trace)
            {
                if (b.Inputs.Count < 1)
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
                
                return new UncurriedFunctionSignature(a, b);
            }

            public IReadOnlyList<Port> Inputs { get; }
            public Port Output { get; }
            public override string ToString() => $"({_a} << {_b}):Function";

            public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
                _a.Call(arguments.Take(_a.Inputs.Count).ToArray(), context)
                  // Now resolve B, skip arguments used in A
                  .Bind(resultOfA => _b.Call(arguments.Skip(_a.Inputs.Count).Prepend(resultOfA).ToArray(), context));
        }
    }
}