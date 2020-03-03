using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public abstract class DeclaredFunction : DeclaredItem, IFunction
    {
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override System.Type[] BodyAlternatives { get; } = {typeof(ExpressionBody), typeof(Scope), typeof(Terminal)};
        public override IType Type => FunctionType.Instance;
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
        Port[] IFunction.Inputs => DeclaredInputs;
        Type IFunction.Output => DeclaredType;
    }

    public class ExtrinsicFunction : DeclaredFunction
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override List<Identifier> ScopeIdentifierWhitelist { get; } = new List<Identifier> {Parser.ReturnIdentifier};

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = ValidateScopeBody(compilationContext);
            if (Body is Terminal)
            {
                compilationContext.LogError(21, $"Non intrinsic function '{Location}' must have a body");
                success = false;
            }

            return success;
        }

        private sealed class ArgumentCaptureScope : ScopeBase
        {
            private readonly IScope _parent;

            public ArgumentCaptureScope(IScope parent, IEnumerable<(Identifier Identifier, IValue Value)> members)
            {
                _parent = parent;
                SetRange(members);
            }

            public override IValue? this[Identifier id, CompilationContext context] => IndexCache(id) ?? _parent[id, context];
        }

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            if (arguments?.Length > 0)
            {
                // If there are any arguments we need to interject a capture scope to store them
                // The capture scope will be indexed before the parent scope when indexing a declared scope
                // Thus the order of indexing for an item becomes "Child -> Captures -> Parent"
                CaptureScope ??= new ArgumentCaptureScope(ParentScope, arguments.Select((arg, index) => (DeclaredInputs[index].Identifier, arg)));
            }

            IValue CompileFunction(ExtrinsicFunction function, IScope parentScope) =>
                function.Body switch
                {
                    // If a function is a binding (has expression body) we just compile the single expression
                    ExpressionBody expressionBody => expressionBody.Expression.ResolveExpression(parentScope, compilationContext),

                    // If a function has a scope body we find the return value
                    Scope scope => scope[Parser.ReturnIdentifier, compilationContext] switch
                    {
                        // If the return value is a function, compile it
                        ExtrinsicFunction returnFunction => CompileFunction(returnFunction, parentScope),
                        null => compilationContext.LogError(7, $"'{Parser.ReturnIdentifier}' not found in function scope"),
                        // TODO: Add support for returning other items as values - structs and namespaces
                        var nyi => throw new NotImplementedException(nyi.ToString())
                    },
                    _ => CompilationErr.Instance
                };


            var callScope = ChildScope ?? CaptureScope ?? ParentScope;
            return arguments.ValidateArguments(DeclaredInputs, callScope, compilationContext)
                       ? CompileFunction(this, callScope)
                       : CompilationErr.Instance;
        }
    }

    public class IntrinsicFunction : DeclaredFunction
    {
        protected override string IntrinsicQualifier => "intrinsic";
        public override bool Validate(CompilationContext compilationContext)
        {
            var success = ImplementingIntrinsic<ICallable>(compilationContext) != null;
            if (!(Body is Terminal))
            {
                compilationContext.LogError(20, $"Intrinsic function '{Location}' cannot have a body");
                success = false;
            }

            return success;
        }

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            ImplementingIntrinsic<ICallable>(compilationContext).Call(arguments, compilationContext);
    }
}