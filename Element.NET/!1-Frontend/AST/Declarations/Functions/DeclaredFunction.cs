namespace Element.AST
{
    public abstract class DeclaredFunction : Declaration, IFunctionSignature
    {
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
        public override IType Type => FunctionType.Instance;
        Port[] IFunctionSignature.Inputs => DeclaredInputs;
        Port IFunctionSignature.Output => DeclaredOutput;
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;
    }
}