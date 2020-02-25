namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class Namespace : DeclaredItem, IValue, IScope
    {
        public override bool Validate(CompilationContext compilationContext) => ValidateScopeBody(compilationContext);
        protected override string Qualifier { get; } = "namespace";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope)};

        private class NamespaceIdentity : IType {}
        IType IValue.Type { get; } = new NamespaceIdentity();

        public IScopeItem? this[Identifier id, CompilationContext compilationContext] => ((IScope) Body)[id, compilationContext];

        IScope? IScope.Parent => Parent;
    }
}