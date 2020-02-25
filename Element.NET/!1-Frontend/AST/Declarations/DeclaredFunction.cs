using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class DeclaredFunction : DeclaredItem<IntrinsicFunction>, IValue, ICallable
    {
        protected override string Qualifier { get; } = string.Empty; // Functions don't have a qualifier
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
        protected override List<Identifier> ScopeIdentifierWhitelist { get; } = new List<Identifier> {Parser.ReturnIdentifier};
        public bool IsNullary => DeclaredInputs == null || DeclaredInputs.Length == 0;

        private class FunctionIdentity : IType {}
        IType IValue.Type { get; } = new FunctionIdentity(); // Functions all share the same type identity. This is because functions are matched to function constraints by shape rather than identity!

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = ValidateIntrinsic(compilationContext) && ValidateScopeBody(compilationContext);
            if (!IsIntrinsic && Body is Terminal)
            {
                compilationContext.LogError(21, $"Non intrinsic function '{Location}' must have a body");
                success = false;
            }

            return success;
        }

        private sealed class CallScope : ScopeBase
        {
            public CallScope(IScope parent, IEnumerable<(Identifier Identifier, IValue Value)> members)
            {
                Parent = parent;
                SetRange(members);
            }

            public override string Location => $"Call to <{Parent.Location}>";
        }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            IValue CompileFunction(DeclaredFunction function, IScope parentScope) =>
                function.Body switch
                {
                    // If a function is a binding (has expression body) we just compile the single expression
                    Binding binding => binding.Expression.ResolveExpression(parentScope, compilationContext),

                    // If a function has a scope body we find the return value
                    Scope scope => scope[Parser.ReturnIdentifier, compilationContext] switch
                    {
                        // If the return value is a function, compile it
                        DeclaredFunction returnFunction => CompileFunction(returnFunction, parentScope),
                        null => compilationContext.LogError(7, $"'{Parser.ReturnIdentifier}' not found in function scope"),
                        // TODO: Add support for returning other items as values - structs and namespaces
                        var nyi => throw new NotImplementedException(nyi.ToString())
                    },
                    _ => CompilationErr.Instance
                };

            if (IsIntrinsic) return GetImplementingIntrinsic(compilationContext)?.Call(arguments, compilationContext);

            var argumentsValid = arguments.ValidateArgumentCount(DeclaredInputs?.Length ?? 0, compilationContext)
                                 && arguments.ValidateArgumentConstraints(DeclaredInputs, Body as IScope ?? Parent, compilationContext);
            return argumentsValid
                ? CompileFunction(this, arguments?.Length > 0
                    // If we have any arguments for this function, push a call scope
                    // else we  parent scope for the function is the declaration's parent
                    ? (IScope) new CallScope(Parent, arguments.Select((arg, index) => (DeclaredInputs[index].Identifier, arg)))
                    : Parent)
                : CompilationErr.Instance;
        }

        public static void ResolveNullary(ref IValue value, CompilationContext compilationContext) =>
            value = value is DeclaredFunction fn && fn.IsNullary
                        ? fn.Call(Array.Empty<IValue>(), compilationContext)
                        : value;

        private Func<TResult> PartialApplication<TInput, TResult>(TInput input, Func<TInput, TResult> func)
        {
            return () => func(input);
        }

        private class InstanceFunction : ICallable, IValue
        {
            public InstanceFunction(IValue value, DeclaredFunction function)
            {
                surrogate = function;
                argument = value;
                Type = (function as IValue).Type;
            }

            private DeclaredFunction surrogate;
            private IValue argument;

            public IType Type { get; }

            public IValue Call(IValue[] arguments, CompilationContext compilationContext)
            {
                if (surrogate.IsNullary)
                {
                    // TODO: Error case if we've indexed a nullary function
                    return CompilationErr.Instance;
                    var call = surrogate.Call(arguments, compilationContext);
                    ResolveNullary(ref call, compilationContext);
                    return call;
                }

                var result = surrogate.Call(arguments.Prepend(argument).ToArray(), compilationContext);
                ResolveNullary(ref result, compilationContext);
                return result;
            }
        }

        public static IValue ResolveAsInstanceFunction(Identifier instanceFunctionIdentifier, IValue structInstance, Struct type, CompilationContext compilationContext)
        {
            if (type[instanceFunctionIdentifier, compilationContext] is DeclaredFunction instanceFunction)
            {
                // TODO: Check instance function has correct first port
                return new InstanceFunction(structInstance, instanceFunction);
            }

            // TODO: Error case
            return CompilationErr.Instance;
        }
    }
}