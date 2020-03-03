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

    public class ExtrinsicFunction : DeclaredFunction, ICompilableFunction
    {
        private bool hasRecursed;

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

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            if (hasRecursed)
            {
                return compilationContext.LogError(11, "Recursion is disallowed");
            }

            if (arguments?.Length > 0)
            {
                // If there are any arguments we need to interject a capture scope to store them
                // The capture scope will be indexed before the parent scope when indexing a declared scope
                // Thus the order of indexing for an item becomes "Child -> Captures -> Parent"
                CaptureScope ??= new ArgumentCaptureScope(ParentScope, arguments.Select((arg, index) => (DeclaredInputs[index].Identifier, arg)));
            }

            hasRecursed = true;

            var callScope = ChildScope ?? CaptureScope ?? ParentScope;
            var result = arguments.ValidateArguments(DeclaredInputs, callScope, compilationContext)
                       ? Compile(callScope, compilationContext)
                       : CompilationErr.Instance;

            CaptureScope = null;
            hasRecursed = false;

            return result;
        }

        public IValue Compile(IScope scope, CompilationContext compilationContext) =>
            scope.CompileFunction(Body, compilationContext);
    }

    internal sealed class ArgumentCaptureScope : ScopeBase
    {
        private readonly IScope _parent;

        public ArgumentCaptureScope(IScope parent, IEnumerable<(Identifier Identifier, IValue Value)> members)
        {
            _parent = parent;
            SetRange(members);
        }

        public override IValue? this[Identifier id, CompilationContext context] => IndexCache(id) ?? _parent[id, context];
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