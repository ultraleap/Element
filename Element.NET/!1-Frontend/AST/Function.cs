using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IFunctionBody {}

    // ReSharper disable once ClassNeverInstantiated.Global
    public class Function : CallableDeclaration<IFunctionBody>, ICallable
    {
        protected override string Qualifier { get; } = string.Empty; // Functions don't have a qualifier
        protected override List<Identifier> ScopeIdentifierWhitelist { get; } = new List<Identifier> {Parser.ReturnIdentifier};
        public bool CanBeCached => Inputs == null || Inputs.Length == 0;

        public override bool Validate(CompilationContext compilationContext) => ValidateIntrinsic(compilationContext) && ValidateBody(compilationContext);

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

            if (IsIntrinsic)
            {
                return ImplementingIntrinsic switch
                {
                    ICallable intrinsic => intrinsic.Call(arguments, compilationContext),
                    {} => compilationContext.LogError(14, $"Found intrinsic '{FullPath}' but it is not callable"),
                    _ => compilationContext.LogError(4, $"Intrinsic '{FullPath}' is not implemented")
                };
            }


            return arguments.ValidateArgumentCount(Inputs?.Length ?? 0, compilationContext)
                   && arguments.ValidateArgumentConstraints(Inputs, FindConstraint, compilationContext)
                // If we have any arguments, push a new temporary scope with them
                // else the parent scope for the function is simply the declaration's parent
                ? CompileFunction(this, arguments?.Length > 0
                    ? Parent.PushTemporaryScope(arguments.Select((arg, index) => (Inputs[index].Identifier, arg)))
                    : Parent)
                : CompilationErr.Instance;
        }
    }
}