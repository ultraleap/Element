using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IFunctionBody {}

    public abstract class CallableDeclaration<TBody> : Item
    {
        [Literal("intrinsic"), Optional] protected string Intrinsic;
        [IndirectLiteral(nameof(Qualifier)), WhitespaceSurrounded] protected Unnamed _;
        [Term] protected Declaration Declaration;
        [Term] protected TBody Body;

        public Port[] Inputs => IsProxied ? ProxiedInputs : Declaration.PortList?.List.ToArray() ?? Array.Empty<Port>();
        public override Identifier Identifier => Declaration.Identifier;
        public bool IsIntrinsic => !string.IsNullOrEmpty(Intrinsic);
        public override string ToString() => Declaration.ToString();
        
        protected abstract string Qualifier { get; }
        protected virtual List<Identifier> ScopeIdentifierWhitelist { get; }
        protected override DeclaredScope Child => Body as Scope;
        protected virtual bool IsProxied => IsIntrinsic;
        protected Port[] ProxiedInputs { get; set; }

        protected bool ValidateBody(CompilationContext compilationContext)
        {
            if (Body is Scope scope)
            {
                return scope.ValidateScope(compilationContext, ScopeIdentifierWhitelist);
            }

            return true;
        }

        protected bool ValidateIntrinsic(CompilationContext compilationContext)
        {
            var success = true;
            if (IsIntrinsic)
            {
                switch (ImplementingIntrinsic)
                {
                    case ICallable callable:
                        ProxiedInputs = callable.Inputs;
                        break;
                    case null: break; // Error already logged by GetImplementingIntrinsic
                    default:
                        compilationContext.LogError(14, $"Found intrinsic '{FullPath}' but it is not callable");
                        success = false;
                        break;
                }
            }

            return success;
        }
    }
    
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

            // If we have any arguments, push a new temporary scope with them
            // else the parent scope for the function is simply the declaration's parent
            return CompileFunction(this, arguments?.Length > 0
                ? Parent.PushTemporaryScope(arguments.Select((arg, index) => (Inputs[index].Identifier, arg)))
                : Parent);
        }
    }
}