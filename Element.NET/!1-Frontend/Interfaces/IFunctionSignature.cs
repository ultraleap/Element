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

        public static IValue ResolveCall(this IFunctionSignature functionSignature, IValue[] arguments, bool allowPartialApplication,
                                         CompilationContext compilationContext)
        {
            if (functionSignature == CompilationError.Instance) return functionSignature;
            var definition = functionSignature.GetDefinition(compilationContext);
            if (compilationContext.ContainsFunction(definition) && !(functionSignature is AppliedFunctionBase)) return compilationContext.LogError(11, $"Multiple references to {functionSignature} in same call stack - Recursion is disallowed");
            compilationContext.PushFunction(definition);
            var tracingThisFunction = false;
            if (functionSignature is IDeclared declared)
            {
                tracingThisFunction = true;
                compilationContext.PushTrace(declared.MakeTraceSite(functionSignature.ToString()));
            }
            

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
                        _ => throw new InternalCompilerException("Function body type unrecognized")
                    };
                
                var callScope = functionSignature switch
                {
                    // If the signature is an applied function we want to use it as the call scope. More arguments are filled by subsequent applications until full application.
                    AppliedFunctionBase appliedFunction => appliedFunction,
                    // Lambdas must be special cased as they can be declared within an expression and may capture function arguments
                    Lambda lambda => lambda.DeclaringScope,
                    // If the signature is a function with a body then we use the bodies parent
                    IFunctionWithBody functionWithBody when functionWithBody.Body is Scope scope => scope.Declarer.Parent,
                    Declaration declaration => declaration.Parent,
                    _ => compilationContext.SourceContext.GlobalScope
                };

                // Fully resolve arguments
                arguments = arguments.Select(arg => arg.FullyResolveValue(compilationContext)).ToArray();
                
                return CheckArguments(functionSignature, arguments, callScope, allowPartialApplication, compilationContext)
                           ? (functionSignature switch
                           {
                               // When there's no arguments we can just resolve immediately
                               IFunctionWithBody functionWithBody when arguments.Length > 0 => new AppliedFunctionWithBody(arguments, functionWithBody, callScope),
                               IFunctionWithBody functionWithBody => functionWithBody.ResolveReturn(callScope, ResolveFunctionBody(functionWithBody.Body, callScope, compilationContext), compilationContext),
                               IFunction function when arguments.Length > 0 => new AppliedFunction(arguments, function, callScope),
                               IFunction function => functionSignature.ResolveReturn(callScope, function.Call(arguments, compilationContext), compilationContext),
                               _ => throw new InternalCompilerException($"{functionSignature} function type not resolvable")
                           }).FullyResolveValue(compilationContext)
                           : CompilationError.Instance;
            }
            finally
            {
                compilationContext.PopFunction();
                if (tracingThisFunction)
                {
                    compilationContext.PopTrace();
                }
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
            var fullyResolvedResult = result.FullyResolveValue(compilationContext);
            var returnConstraint = functionSignature.Output.ResolveConstraint(callScope, compilationContext);
            return returnConstraint.MatchesConstraint(fullyResolvedResult, compilationContext)
                       ? fullyResolvedResult
                       : compilationContext.LogError(8, $"Result '{fullyResolvedResult}' for function '{functionSignature.GetDefinition(compilationContext)}' does not match '{returnConstraint}' constraint");
        }
        
        private abstract class AppliedFunctionBase : ScopeBase, IFunctionSignature
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

            Port[] IFunctionSignature.Inputs => _inputs;
            Port IFunctionSignature.Output => _definition.Output;

            public override string ToString() => _definition.ToString();

            protected virtual IScope? _child => null;

            public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
                _child?[id, false, compilationContext] ?? IndexCache(id) ?? (recurse ? _parent[id, true, compilationContext] : null);

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

        public static IFunctionSignature Uncurry(this IFunctionSignature a, IFunctionSignature b,
                                                 Context context) =>
            UncurriedFunction.Create(a, b, context);

        private class UncurriedFunction : IFunction
        {
            private readonly IFunctionSignature _a;
            private readonly IFunctionSignature _b;

            private UncurriedFunction(IFunctionSignature a, IFunctionSignature b)
            {
                _a = a;
                _b = b;
                Inputs = _a.Inputs.Concat(_b.Inputs.Skip(1)).ToArray();
                Output = _b.Output;
            }
            
            public static IFunctionSignature Create(IFunctionSignature a, IFunctionSignature b, Context context)
            {
                if (b.Inputs.Length < 1)
                {
                    return context.LogError(23, $"Function B '{b}' must have at least 1 input and where the first input must be compatible with the output of Function A");
                }

                if (a.Inputs.Any(p => p == Port.VariadicPort))
                {
                    return context.LogError(23, $"Function A '{a}' is variadic - variadic functions cannot be the first argument of an uncurrying operation");
                }

                if (a.Inputs.Concat(b.Inputs).Any(p => p?.Identifier == null))
                {
                    return context.LogError(23, "Cannot uncurry functions with discarded/unnamed ports");
                }

                foreach (var aPort in a.Inputs)
                {
                    // TODO: Better solution to this error, it's very annoying to not be allowed to uncurry functions just because of port names!
                    Identifier? id = null;
                    if (b.Inputs.Skip(1).Any(bPort => aPort.Identifier.Equals(id = bPort.Identifier)))
                    {
                        return context.LogError(23, $"'{id}' is defined on both functions, these functions cannot be uncurried");
                    }
                }
                
                return new UncurriedFunction(a, b);
            }

            IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;
            public Port[] Inputs { get; }
            public Port Output { get; }
            public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
                _b.ResolveCall(
                    // Skip arguments meant for function a
                    arguments.Skip(_a.Inputs.Length)
                             .Prepend(_a.ResolveCall(arguments.Take(_a.Inputs.Length).ToArray(), false, compilationContext))
                             .ToArray(),
                    false, compilationContext);
        }
    }
}