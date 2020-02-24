namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class Namespace : DeclaredItem<Intrinsic>, IValue, IScope
    {
        public override bool Validate(CompilationContext compilationContext) => ValidateScopeBody(compilationContext);
        protected override string Qualifier { get; } = "namespace";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope)};
        public string TypeIdentity => FullPath;

        public IScopeItem? this[Identifier id] => ((IScope) Body)[id];

        IScope? IScope.Parent => Parent;
    }
}