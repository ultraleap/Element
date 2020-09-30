using System.Collections.Generic;

namespace Element.AST
{
    public class Namespace : Value
    {
        private readonly ResolvedBlock _resolvedBlock;
        public Namespace(ResolvedBlock resolvedBlock) => _resolvedBlock = resolvedBlock;
        public override IReadOnlyList<Identifier> Members => _resolvedBlock.Members;
        public override Result<IValue> Index(Identifier id, Context context) => _resolvedBlock.Index(id, context);
    }
}