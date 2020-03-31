namespace Element.AST
{
    public abstract class DeclaredFunction : Declaration, IFunction
    {
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
        public override IType Type => FunctionType.Instance;
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
        Port[] IFunction.Inputs => DeclaredInputs;
        Port IFunction.Output => DeclaredOutput;
    }

    public class ExtrinsicFunction : DeclaredFunction, ICompilableFunction
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override Identifier[] ScopeIdentifierWhitelist { get; } = {Parser.ReturnIdentifier};

        internal override bool Validate(SourceContext sourceContext)
        {
            var success = base.Validate(sourceContext);
            if (Body is Terminal)
            {
                sourceContext.LogError(21, $"Non intrinsic function '{Location}' must have a body");
                success = false;
            }

            return success;
        }

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            this.ApplyArguments(arguments, DeclaredInputs, DeclaredOutput, Body, ChildScope ?? ParentScope, compilationContext);

        public IValue Compile(IScope scope, CompilationContext compilationContext) =>
            scope.CompileFunction(Body, compilationContext);

        public ICompilableFunction Definition => this;
    }

    public class IntrinsicFunction : DeclaredFunction
    {
        protected override string IntrinsicQualifier => "intrinsic";

        internal override bool Validate(SourceContext sourceContext)
        {
            var success = ImplementingIntrinsic<IFunction>(sourceContext) != null;
            if (!(Body is Terminal))
            {
                sourceContext.LogError(20, $"Intrinsic function '{Location}' cannot have a body");
                success = false;
            }

            return success;
        }

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            ImplementingIntrinsic<ICallable>(compilationContext).Call(arguments, compilationContext);
    }
}