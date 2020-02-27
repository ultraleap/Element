namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Namespace : DeclaredItem, IValue, IScope
    {
        public override bool Validate(CompilationContext compilationContext) => ValidateScopeBody(compilationContext);
        protected override string Qualifier { get; } = "namespace";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope)};

        IType IValue.Type { get; } = NamespaceType.Instance;

        public IScopeItem? this[Identifier id, CompilationContext compilationContext] => ((IScope) Body)[id, compilationContext];

        IScope? IScope.Parent => Parent;
    }
}