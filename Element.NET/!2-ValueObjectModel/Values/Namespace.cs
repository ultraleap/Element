using System.Collections.Generic;
using ResultNET;

namespace Element.AST
{
    public class Namespace : Value
    {
        private readonly ResolvedBlock _resolvedBlock;
        public static Result<IValue> Create(NamespaceBlock block, IScope parent, Context context)
        {
            Namespace? result = null;
            return block.ResolveBlock(parent, context, () => result)
                        .Map(resolvedScope => (IValue)(result = new Namespace(resolvedScope)));
        }
        private Namespace(ResolvedBlock resolvedBlock) => _resolvedBlock = resolvedBlock;
        public override IReadOnlyList<Identifier> Members => _resolvedBlock.Members;
        public override Result<IValue> Index(Identifier id, Context context) => _resolvedBlock.Index(id, context);
    }
}