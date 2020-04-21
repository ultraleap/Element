using System;
using System.Linq;

namespace Element.AST
{
    public interface IUnique<T> where T : IValue
    {
        T GetDefinition(CompilationContext compilationContext);
    }
    
    public interface IFunctionSignature : IValue, IUnique<IFunctionSignature>
    {
        Port[] Inputs { get; }
        Port Output { get; }
    }

    public interface IFunction : IFunctionSignature
    {
        IValue Call(IValue[] arguments, CompilationContext compilationContext);
    }

    public interface IFunctionWithBody : IFunctionSignature // Not IFunction as all functions with bodies are resolved identically in ResolveCall
    {
        object Body { get; }
    }

    public static class FunctionExtensions
    {
        public static bool IsNullary(this IFunctionSignature functionSignature) => functionSignature.Inputs.Length == 0;
        public static IValue ResolveNullaryFunction(this IValue value, CompilationContext compilationContext)
        {
            var previous = value;
            while (previous is IFunctionSignature fn && fn.IsNullary())
            {
                var result = fn.ResolveCall(Array.Empty<IValue>(), false, compilationContext);
                // ReSharper disable once PossibleUnintendedReferenceComparison
                if (result == previous) break; // Prevent infinite loop if a nullary just returns itself
                previous = result;
            }

            return previous;
        }

        public static IValue ResolveCall(this IFunctionSignature functionSignature, IValue[] arguments, bool allowPartialApplication,
                                         CompilationContext compilationContext)
        {
            var definition = functionSignature.GetDefinition(compilationContext);
            if (compilationContext.ContainsFunction(definition) && !(functionSignature is AppliedFunctionBase)) return compilationContext.LogError(11, $"Multiple references to {functionSignature} in same call stack - Recursion is disallowed");
            compilationContext.PushFunction(definition);

            try
            {
                // Local function since it can call into itself recursively
                static IValue ResolveFunctionBody(object body, IScope scope, CompilationContext compilationContext)  =>
                    body switch
                    {
                        // If a function has expression body we just compile the single expression using the call scope
                        ExpressionBody exprBody => exprBody.Expression.ResolveExpression(scope, compilationContext),
                        // If a function has a scope body we need to find the return identifier
                        IScope scopeBody => scopeBody[Parser.ReturnIdentifier, false, compilationContext] switch
                        {
                            IFunctionWithBody nullaryReturn when nullaryReturn.IsNullary() => ResolveFunctionBody(nullaryReturn.Body, scope, compilationContext),
                            IFunctionWithBody functionReturn => functionReturn,
                            null => compilationContext.LogError(7, $"'{Parser.ReturnIdentifier}' not found in function scope"),
                            var nyi => throw new NotImplementedException(nyi.ToString())
                        },
                        _ => CompilationErr.Instance
                    };
                
                var callScope = functionSignature switch
                {
                    // If the signature is an applied function we want to use it as the call scope. More arguments are filled by subsequent applications until full application.
                    AppliedFunctionBase appliedFunction => appliedFunction,
                    
                    AnonymousFunction anonymousFunction => anonymousFunction.Parent,
                    
                    // If the signature is a function with a body then we can use the body 
                    IFunctionWithBody functionWithBody when functionWithBody.Body is IDeclared declared => declared.Declarer.ChildScope ?? declared.Declarer.ParentScope,
                    Declaration declaration => declaration.ChildScope ?? declaration.ParentScope,
                    _ => compilationContext.SourceContext.GlobalScope
                };
                
                return CheckArguments(functionSignature, arguments, callScope, allowPartialApplication, compilationContext)
                           ? (functionSignature switch
                                 {
                                     // When there's no arguments we can just resolve immediately
                                     IFunctionWithBody functionWithBody when arguments.Length > 0 => new AppliedFunctionWithBody(arguments, functionWithBody, callScope),
                                     IFunctionWithBody functionWithBody => functionWithBody.ResolveReturn(callScope, ResolveFunctionBody(functionWithBody.Body, callScope, compilationContext), compilationContext),
                                     IFunction function when arguments.Length > 0 => new AppliedFunction(arguments, function, callScope),
                                     IFunction function => functionSignature.ResolveReturn(callScope, function.Call(arguments, compilationContext), compilationContext),
                                     _ => throw new InternalCompilerException($"{functionSignature} function type not resolvable")
                                 }).ResolveNullaryFunction(compilationContext)
                           : CompilationErr.Instance;
            }
            finally
            {
                compilationContext.PopFunction();
            }
        }

        private static bool CheckArguments(IFunctionSignature functionSignature, IValue[] arguments, IScope callScope, bool allowPartialApplication, CompilationContext compilationContext)
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
                    var constraint = port.ResolveConstraint(callScope, compilationContext);
                    if (!constraint.MatchesConstraint(arg, compilationContext))
                    {
                        compilationContext.LogError(8, $"Value '{arg}' given for port '{port}' does not match '{constraint}' constraint");
                        argumentsValid = false;
                    }
                }
            }

            return argumentsValid;
        }

        private static IValue ResolveReturn(this IFunctionSignature functionSignature, IScope callScope, IValue result, CompilationContext compilationContext)
        {
            var returnConstraint = functionSignature.Output.ResolveConstraint(callScope, compilationContext);
            return !returnConstraint.MatchesConstraint(result, compilationContext)
                       ? compilationContext.LogError(8, $"Result '{result}' for function '{functionSignature.GetDefinition(compilationContext)}' does not match '{returnConstraint}' constraint")
                       : result switch
                       {
                           Element.Expression expr => (IValue)ConstantFolding.Optimize(expr),
                           _ => result
                       };
        }
        
        private abstract class AppliedFunctionBase : ScopeBase<IValue>, IFunctionSignature
        {
            protected AppliedFunctionBase(IValue[] arguments, IFunctionSignature definition, IScope parent)
            {
                _definition = definition;
                _parent = parent;
                _inputs = definition.Inputs.Skip(arguments.Length).ToArray();
                SetRange(arguments.WithoutDiscardedArguments(definition.Inputs));
            }

            private readonly IScope _parent;
            private readonly Port[] _inputs;
            private readonly IFunctionSignature _definition;

            IType IValue.Type => _definition.Type;
            Port[] IFunctionSignature.Inputs => _inputs;
            Port IFunctionSignature.Output => _definition.Output;

            protected virtual IScope? _child => null;

            public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
                _child != null
                    ? _child[id, false, compilationContext] ?? IndexCache(id) ?? (recurse ? _parent[id, true, compilationContext] : null)
                    : IndexCache(id) ?? (recurse ? _parent[id, true, compilationContext] : null);

            public IFunctionSignature GetDefinition(CompilationContext compilationContext) => _definition;
        }

        private class AppliedFunction : AppliedFunctionBase, IFunction
        {
            public AppliedFunction(IValue[] arguments, IFunction definition, IScope parent)
                : base(arguments, definition, parent) =>
                _definition = definition;

            private readonly IFunction _definition;

            IValue IFunction.Call(IValue[] arguments, CompilationContext compilationContext) =>
                this.ResolveReturn(this, _definition.Call(this.Concat(arguments).ToArray(), compilationContext), compilationContext);
        }

        private class AppliedFunctionWithBody : AppliedFunctionBase, IFunctionWithBody
        {
            public AppliedFunctionWithBody(IValue[] arguments, IFunctionWithBody functionWithBody, IScope parent)
                : base(arguments, functionWithBody, parent)
            {
                Body = functionWithBody.Body switch
                {
                    ExpressionBody exprBody => exprBody,
                    IScope scopeBody => scopeBody.Clone(this),
                    _ => throw new InternalCompilerException("Cannot create function instance as function body type is not recognized")
                };
            }

            protected override IScope? _child => Body as IScope;

            public object Body { get; }
        }
    }
}