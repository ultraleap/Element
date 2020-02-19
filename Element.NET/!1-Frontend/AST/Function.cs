using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IFunctionBody {}
    
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Function : Item, ICallable
    {
        [Literal("intrinsic"), Optional] private string _intrinsic;
        [Term] private Declaration _declaration;
        [Term] private IFunctionBody _functionBody;

        public override Identifier Identifier => _declaration.Identifier;
        protected override DeclaredScope Child => _functionBody as Scope;
        public Port[] Inputs => _declaration.PortList?.List.ToArray() ?? Array.Empty<Port>();
        public bool CanBeCached => Inputs == null || Inputs.Length == 0;
        public bool IsIntrinsic => !string.IsNullOrEmpty(_intrinsic);
        public override string ToString() => _declaration.ToString();

        private readonly List<Identifier> _functionIdWhitelist = new List<Identifier> {Parser.ReturnIdentifier};

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = true;
            if (_functionBody is Scope scope)
            {
                success &= scope.ValidateScope(compilationContext, _functionIdWhitelist);
            }

            return success;
        }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            IValue CompileFunction(Function function, IScope parentScope) =>
                function._functionBody switch
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

            // If we have any arguments, push a new temporary scope with them
            // else the parent scope for the function is simply the declaration's parent
            return CompileFunction(this, arguments?.Length > 0
                ? Parent.PushTemporaryScope(arguments.Select((arg, index) => (Inputs[index].Identifier, arg)))
                : Parent);
        }
    }
}