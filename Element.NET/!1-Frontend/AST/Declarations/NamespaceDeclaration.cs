using System.Collections.Generic;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class NamespaceDeclaration : Declaration
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override string Qualifier { get; } = "namespace";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Block)};
        public override string ToString() => $"{Location}:Namespace";
        public override Result<IValue> Index(Identifier id, CompilationContext context) => Child!.Index(id, context);
        public override IReadOnlyList<IValue> Members => Child!.Members;
    }
}