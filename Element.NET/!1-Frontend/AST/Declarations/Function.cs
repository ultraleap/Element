using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IFunctionBody {}

    // ReSharper disable once ClassNeverInstantiated.Global
    public class Function : DeclaredCallable<IFunctionBody, IntrinsicFunction>, IValue, ICallable
    {
        protected override string Qualifier { get; } = string.Empty; // Functions don't have a qualifier
        protected override List<Identifier> ScopeIdentifierWhitelist { get; } = new List<Identifier> {Parser.ReturnIdentifier};
        private bool IsNullary => DeclaredInputs == null || DeclaredInputs.Length == 0;
        public IConstraint Identity { get; } = null; // TODO: Add function identity

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

            return IsIntrinsic
                       ? GetImplementingIntrinsic(compilationContext)?.Call(arguments, compilationContext)
                       : arguments.ValidateArgumentCount(DeclaredInputs?.Length ?? 0, compilationContext)
                         && arguments.ValidateArgumentConstraints(DeclaredInputs, FindConstraint, compilationContext)
                           // If we have any arguments, push a new temporary scope with them
                           // else the parent scope for the function is simply the declaration's parent
                           ? CompileFunction(this, arguments?.Length > 0
                                                       ? Parent.PushTemporaryScope(arguments.Select((arg, index) => (DeclaredInputs[index].Identifier, arg)))
                                                       : Parent)
                           : CompilationErr.Instance;
        }

        public static void ResolveNullary(ref IValue value, CompilationContext compilationContext) =>
            value = value is Function fn && fn.IsNullary
                        ? fn.Call(Array.Empty<IValue>(), compilationContext)
                        : value;
    }
}