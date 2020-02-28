namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Namespace : DeclaredItem, IScope
    {
        public override bool Validate(CompilationContext compilationContext) => ValidateScopeBody(compilationContext);
        protected override string Qualifier { get; } = "namespace";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope)};
        public override IType Type => NamespaceType.Instance;
        public IValue? this[Identifier id, CompilationContext compilationContext] => Child[id, compilationContext];
    }
}