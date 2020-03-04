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

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            this.ResolveCall(arguments, ref CaptureScope, ref hasRecursed, DeclaredInputs, ChildScope, ParentScope, compilationContext);

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