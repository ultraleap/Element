using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Function : DeclaredCallable<IntrinsicFunction>, IValue, ICallable
    {
        protected override string Qualifier { get; } = string.Empty; // Functions don't have a qualifier
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
        protected override List<Identifier> ScopeIdentifierWhitelist { get; } = new List<Identifier> {Parser.ReturnIdentifier};
        private bool IsNullary => DeclaredInputs == null || DeclaredInputs.Length == 0;
        public string TypeIdentity { get; } = null; // TODO: Add function identity

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = ValidateIntrinsic(compilationContext) && ValidateScopeBody(compilationContext);
            if (!IsIntrinsic && Body is Terminal)
            {
                compilationContext.LogError(21, $"Non intrinsic function '{FullPath}' must have a body");
                success = false;
            }

            return success;
        }

        private sealed class CallScope : ScopeBase
        {
            public CallScope(IScope parent, IEnumerable<(Identifier Identifier, IValue Value)> members)
            {
                Parent = parent;
                AddRange(members);
            }
        }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            IValue CompileFunction(Function function, IScope parentScope) =>
                function.Body switch
                {
                    // If a function is a binding (has expression body) we just compile the single expression
                    Binding binding => binding.Expression.ResolveExpression(parentScope, compilationContext),

                    // If a function has a scope body we find the return value
                    Scope scope => scope[Parser.ReturnIdentifier] switch
                    {
                        // If the return value is a function, compile it
                        Function returnFunction => CompileFunction(returnFunction, parentScope),
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
            value = value is Function fn && fn.IsNullary
                        ? fn.Call(Array.Empty<IValue>(), compilationContext)
                        : value;
    }
}