using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class DeclaredFunction : DeclaredItem, IFunction
    {
        protected override string Qualifier { get; } = string.Empty; // Functions don't have a qualifier
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
        protected override List<Identifier> ScopeIdentifierWhitelist { get; } = new List<Identifier> {Parser.ReturnIdentifier};
        public override IType Type => FunctionType.Instance;
        Port[] IFunction.Inputs => DeclaredInputs;
        Type IFunction.Output => DeclaredType; 

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = ValidateIntrinsic<ICallable>(compilationContext) && ValidateScopeBody(compilationContext);
            if (!IsIntrinsic && Body is Terminal)
            {
                compilationContext.LogError(21, $"Non intrinsic function '{Location}' must have a body");
                success = false;
            }

            return success;
        }

        private sealed class CallScope : ScopeBase
        {
            private readonly IScope _parent;

            public CallScope(IScope parent, IEnumerable<(Identifier Identifier, IValue Value)> members)
            {
                _parent = parent;
                SetRange(members);
            }

            //public override string Location => $"Call to <{Parent.Location}>";
            public override IValue? this[Identifier id, CompilationContext context] => IndexCache(id) ?? _parent[id, context];
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

            if (IsIntrinsic) return IntrinsicCache.GetIntrinsic<ICallable>(Location, compilationContext)?.Call(arguments, compilationContext);

            return arguments.ValidateArguments(DeclaredInputs, Body as IScope ?? Parent, compilationContext)
                       ? CompileFunction(this, arguments?.Length > 0
                                                   // If we have any arguments for this function, push a call scope
                                                   // else we  parent scope for the function is the declaration's parent
                                                   ? (IScope) new CallScope(Parent, arguments.Select((arg, index) => (DeclaredInputs[index].Identifier, arg)))
                                                   : Parent)
                       : CompilationErr.Instance;
        }
    }
}