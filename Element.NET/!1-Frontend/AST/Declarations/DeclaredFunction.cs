using System.Collections.Generic;

namespace Element.AST
{
    public abstract class DeclaredFunction : Declaration, IFunction
    {
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
        public override IType Type => FunctionType.Instance;
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
        Port[] IFunction.Inputs => DeclaredInputs;
        Type IFunction.Output => DeclaredType;
    }

    public class ExtrinsicFunction : DeclaredFunction, ICompilableFunction
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

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            this.ApplyArguments(arguments, DeclaredInputs, Body, ChildScope ?? ParentScope, compilationContext);

        public IValue Compile(IScope scope, CompilationContext compilationContext) =>
            scope.CompileFunction(Body, compilationContext);

        public bool IsBeingCompiled { get; set; }
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