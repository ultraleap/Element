namespace Element.AST
{
    public class NamespaceDeclaration : Declaration
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override string Qualifier { get; } = "namespace";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Block)};
        protected override Result<IValue> ResolveImpl(IScope scope, CompilationContext context) =>
            ((Block) Body).Resolve(scope, context)
                          .Map(resolvedScope => (IValue)new Namespace(resolvedScope, Identifier, context.CurrentDeclarationLocation));
    }
}