using System;
using System.Linq;

namespace Element.AST
{
    public interface IFunctionSignature : IValue
    {
        Port[] Inputs { get; }
        Port Output { get; }
        IFunctionSignature GetDefinition(CompilationContext compilationContext);
    }

    public interface IFunction : IFunctionSignature
    {
        IValue Call(IValue[] arguments, CompilationContext compilationContext);
    }

    public interface IFunctionWithBody : IFunction
    {
        object Body { get; }
    }

    public static class FunctionExtensions
    {
        public static bool IsNullary(this IFunctionSignature functionSignature) => functionSignature.Inputs.Length == 0;
        public static IValue ResolveNullaryFunction(this IValue value, CompilationContext compilationContext)
        {
            var previous = value;
            while (previous is IFunction fn && fn.IsNullary())
            {
                var result = fn.Call(Array.Empty<IValue>(), compilationContext);
                // ReSharper disable once PossibleUnintendedReferenceComparison
                if (result == previous) break; // Prevent infinite loop if a nullary just returns itself
                previous = result;
            }

            return previous;
        }

        public static IValue ResolveCall(this IFunctionSignature functionSignature, IValue[] arguments, IScope callScope, bool allowPartialApplication,
                                         CompilationContext compilationContext)
        {
            // If we're resolving a partially applied function then we need to remember to use it's scope
            if (functionSignature is AppliedFunction appliedFunction) callScope = appliedFunction;
            return CheckArguments(functionSignature, arguments, allowPartialApplication, compilationContext)
                       ? (functionSignature switch
                             {
                                 // When there's no arguments we can just resolve immediately
                                 IFunctionWithBody functionWithBody when arguments.Length > 0 => new AppliedFunctionWithBody(arguments, functionWithBody, callScope),
                                 IFunctionWithBody functionWithBody => functionWithBody.ResolveReturn(functionWithBody.ResolveFunctionBody(callScope, compilationContext), compilationContext),
                                 IFunction callable when arguments.Length > 0 => new AppliedFunction(arguments, callable, callScope),
                                 IFunction callable => functionSignature.ResolveReturn(() => callable.Call(arguments, compilationContext), compilationContext),
                                 _ => throw new InternalCompilerException($"{functionSignature} function type not resolvable")
                             }).ResolveNullaryFunction(compilationContext)
                       : CompilationErr.Instance;
        }

        private static bool CheckArguments(IFunctionSignature functionSignature, IValue[] arguments, bool allowPartialApplication, CompilationContext compilationContext)
        {
            var argumentsValid = true;
            if (arguments.Length > 0)
            {
                var ports = functionSignature.Inputs;
                var isVariadic = ports.Any(p => p == Port.VariadicPort);
                if (!allowPartialApplication && !isVariadic && arguments.Length != ports.Length
                    || !allowPartialApplication && arguments.Length < ports.Length)
                {
                    compilationContext.LogError(6, $"Expected '{ports.Length}' arguments but got '{arguments.Length}'");
                    argumentsValid = false;
                }

                for (var i = 0; i < Math.Min(ports.Length, arguments.Length); i++)
                {
                    var arg = arguments[i];
                    var port = ports[i];
                    if (port == Port.VariadicPort)
                        break; // If we find a variadic port we can't do any more constraint checking here, it must be done within the functions implementation.
                    var constraint = port.ResolveConstraint(compilationContext);
                    if (!constraint.MatchesConstraint(arg, compilationContext))
                    {
                        compilationContext.LogError(
                            8, $"Value '{arg}' given for port '{port}' does not match '{constraint}' constraint");
                        argumentsValid = false;
                    }
                }
            }

            return argumentsValid;
        }

        private static IValue ResolveReturn(this IFunctionSignature functionSignature, Func<IValue> resolveReturn, CompilationContext compilationContext)
        {
            var definition = functionSignature.GetDefinition(compilationContext);
            if (compilationContext.ContainsFunction(definition)) return compilationContext.LogError(11, $"Multiple references to {functionSignature} in same call stack - Recursion is disallowed");
            compilationContext.PushFunction(definition);

            try
            {
                var result = resolveReturn();
                var returnConstraint = functionSignature.Output.ResolveConstraint(compilationContext);
                return !returnConstraint.MatchesConstraint(result, compilationContext)
                           ? compilationContext.LogError(8, $"Result '{result}' for function '{definition}' does not match '{returnConstraint}' constraint")
                           : result switch
                           {
                               Element.Expression expr => (IValue)ConstantFolding.Optimize(expr),
                               _ => result
                           };
            }
            finally
            {
                compilationContext.PopFunction();
            }
        }

        private static Func<IValue> ResolveFunctionBody(this IFunctionWithBody functionWithBody, IScope callScope, CompilationContext compilationContext)
        {
            // Local function since it can call into itself recursively
            static IValue ResolveFunctionBody(IScope callScope, object body, CompilationContext compilationContext)  =>
                body switch
                {
                    // If a function has expression body we just compile the single expression using the call scope
                    ExpressionBody exprBody => exprBody.Expression.ResolveExpression(callScope, compilationContext),
                    // If a function has a scope body we need to find the return identifier
                    IScope scopeBody => scopeBody[Parser.ReturnIdentifier, false, compilationContext] switch
                    {
                        IFunctionWithBody nullaryReturn when nullaryReturn.IsNullary() => ResolveFunctionBody(scopeBody, nullaryReturn.Body, compilationContext),
                        IFunctionWithBody functionReturn => functionReturn,
                        null => compilationContext.LogError(7, $"'{Parser.ReturnIdentifier}' not found in function scope"),
                        var nyi => throw new NotImplementedException(nyi.ToString())
                    },
                    _ => CompilationErr.Instance
                };

            return () => ResolveFunctionBody(callScope, functionWithBody.Body, compilationContext);
        }
        
        private class AppliedFunction : ScopeBase<IValue>, IFunction
        {
            public AppliedFunction(IValue[] arguments, IFunction definition, IScope parent)
            {
                _definition = definition;
                _parent = parent;
                _inputs = definition.Inputs.Skip(arguments.Length).ToArray();
                SetRange(arguments.WithoutDiscardedArguments(definition.Inputs));
            }

            private readonly IScope _parent;
            private readonly IFunction _definition;
            private readonly Port[] _inputs;

            IType IValue.Type => _definition.Type;
            Port[] IFunctionSignature.Inputs => _inputs;
            Port IFunctionSignature.Output => _definition.Output;
            IFunctionSignature IFunctionSignature.GetDefinition(CompilationContext compilationContext) => _definition;

            public virtual IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
                this.ResolveReturn(() => _definition.Call(this.Concat(arguments).ToArray(), compilationContext), compilationContext);

            public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
                IndexCache(id) ?? (recurse ? _parent[id, true, compilationContext] : null);
        }

        private class AppliedFunctionWithBody : AppliedFunction, IFunctionWithBody
        {
            public AppliedFunctionWithBody(IValue[] arguments, IFunctionWithBody definition, IScope parent)
                : base(arguments, definition, parent)
            {
                Body = definition.Body switch
                {
                    ExpressionBody b => b, // No need to clone expression bodies
                    Scope scopeBody => scopeBody.Clone(this),
                    _ => throw new InternalCompilerException("Cannot create function instance as function body type is not recognized")
                };
            }

            public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
                this.ResolveReturn(this.ResolveFunctionBody(this, compilationContext), compilationContext);

            public object Body { get; }
        }
    }
}