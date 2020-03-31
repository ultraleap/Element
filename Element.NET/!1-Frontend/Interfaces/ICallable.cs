using System;
using System.Linq;

namespace Element.AST
{
    public interface ICallable : IValue
    {
        IValue Call(IValue[] arguments, CompilationContext compilationContext);
    }

    public interface IFunction : ICallable
    {
        Port[] Inputs { get; }
        Port Output { get; }
    }

    public interface ICompilableFunction : IFunction
    {
        IValue Compile(IScope scope, CompilationContext compilationContext);
        ICompilableFunction Definition { get; }
    }

    public static class FunctionExtensions
    {
        public static bool IsNullary(this IFunction function) => function.Inputs.Length == 0;
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

        public static IValue ApplyArguments(this ICompilableFunction function, IValue[] arguments, Port[] inputs, Port returnPort, object body, IScope callScope, CompilationContext compilationContext) =>
            !(arguments.Length > 0) || arguments.ValidateArguments(inputs, callScope, compilationContext)
                ? arguments.Length > 0
                      ? new AppliedFunction(arguments, function, callScope, body).ResolveNullaryFunction(compilationContext)
                      : ResolveCall(function, callScope, returnPort, compilationContext)
                : CompilationErr.Instance;

        private static IValue ResolveCall(ICompilableFunction function, IScope callScope, Port returnPort, CompilationContext compilationContext)
        {
            if (compilationContext.ContainsFunction(function.Definition)) return compilationContext.LogError(11, $"Multiple references to {function} in same call stack - Recursion is disallowed");
            compilationContext.PushFunction(function.Definition);

            try
            {
                var result = function.Compile(callScope, compilationContext);
                var returnConstraint = returnPort.ResolveConstraint(compilationContext);
                return !returnConstraint.MatchesConstraint(result, compilationContext)
                           ? compilationContext.LogError(8, $"Result '{result}' for function '{function.Definition}' does not match '{returnConstraint}' constraint")
                           : result;
            }
            finally
            {
                compilationContext.PopFunction();
            }
        }

        private class AppliedFunction : ScopeBase, ICompilableFunction
        {
            public AppliedFunction(IValue[] arguments, ICompilableFunction definition, IScope parent, object body)
            {
                Definition = definition;
                _parent = parent;
                _body = body switch
                {
                    ExpressionBody b => b, // No need to clone expression bodies
                    Scope scopeBody => scopeBody.Clone(this),
                    _ => throw new InternalCompilerException("Cannot create function instance as function body type is not recognized")
                };

                Inputs = definition.Inputs.Skip(arguments.Length).ToArray();
                SetRange(arguments.WithoutDiscardedArguments(definition.Inputs));
            }

            private readonly IScope _parent;
            private readonly object _body;
            public IType Type => Definition.Type;
            public Port[] Inputs { get; }
            public Port Output => Definition.Output;

            public IValue Compile(IScope scope, CompilationContext compilationContext) => scope.CompileFunction(_body, compilationContext);
            public ICompilableFunction Definition { get; }

            public IValue Call(IValue[] _, CompilationContext compilationContext) => ResolveCall(this, this, Output, compilationContext);

            public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
                IndexCache(id) ?? (recurse ? _parent[id, true, compilationContext] : null);
        }
    }
}