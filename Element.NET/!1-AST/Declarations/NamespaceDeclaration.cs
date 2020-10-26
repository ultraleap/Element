using System;
using System.Linq;

namespace Element.AST
{
    public class NamespaceDeclaration : Declaration
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override string Qualifier { get; } = "namespace";
        protected override Type[] BodyAlternatives { get; } = {typeof(NamespaceBlock)};
        protected override Result<IValue> ResolveImpl(IScope scope, Context context) => Namespace.Create(((NamespaceBlock) Body), scope, context);
        protected override void ValidateDeclaration(ResultBuilder builder, Context context)
        {
            PortList?.Validate(builder, context);
            ReturnConstraint?.Validate(builder, context);
            if (Body is NamespaceBlock block)
            {
                block.Validate(builder, context);
                block.ValidateIdentifiers(builder);
            }
        }
    }
    
    public class NamespaceBlock : DeclarationBlock
    {
        public void ValidateIdentifiers(ResultBuilder builder)
        {
            foreach (var decl in Items ?? Enumerable.Empty<Declaration>())
            {
                decl.Identifier.Validate(builder,Array.Empty<Identifier>(), Array.Empty<Identifier>());
            }
        }
    }
}