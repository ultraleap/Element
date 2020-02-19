using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IFunctionBody {}

    public abstract class CallableDeclaration<TBody> : Item
    {
        [field: Literal("intrinsic"), Optional] public string Intrinsic { get; }
        [IndirectLiteral(nameof(Qualifier)), WhitespaceSurrounded] private Unnamed _;
        [field: Term] public Declaration Declaration { get; }
        [field: Term] public TBody Body { get; }

        protected abstract string Qualifier { get; }
        public Port[] Inputs => IsProxied ? ProxiedInputs : Declaration.PortList?.List.ToArray() ?? Array.Empty<Port>();
        protected virtual bool IsProxied => IsIntrinsic;
        protected Port[] ProxiedInputs { get; set; }

        public override Identifier Identifier => Declaration.Identifier;
        protected override DeclaredScope Child => Body as Scope;
        public bool IsIntrinsic => !string.IsNullOrEmpty(Intrinsic);
        public override string ToString() => Declaration.ToString();

        protected bool ValidateBody(CompilationContext compilationContext)
        {
            if (Body is Scope scope)
            {
                return scope.ValidateScope(compilationContext);
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

            if (!IsIntrinsic && Inputs.Length < 1)
            {
                compilationContext.LogError(13, $"Non intrinsic '{FullPath}' must have ports");
                success = false;
            }

            return success;
        }
    }
    
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Function : CallableDeclaration<IFunctionBody>, ICallable
    {
        protected override string Qualifier { get; } = string.Empty; // Functions don't have a qualifier
        public bool CanBeCached => Inputs == null || Inputs.Length == 0;

        private readonly List<Identifier> _functionIdWhitelist = new List<Identifier> {Parser.ReturnIdentifier};

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