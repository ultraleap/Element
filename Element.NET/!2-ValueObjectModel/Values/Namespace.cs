using System.Collections.Generic;

namespace Element.AST
{
    public class Namespace : Value
    {
        private readonly ResolvedBlock _resolvedBlock;

        public Namespace(Identifier identifier, ResolvedBlock resolvedBlock)
        {
            Identifier = identifier;
            _resolvedBlock = resolvedBlock;
        }

        public override Identifier? Identifier { get; }
        public override IReadOnlyList<Identifier> Members => _resolvedBlock.Members;
        public override Result<IValue> Index(Identifier id, CompilationContext context) => _resolvedBlock.Index(id, context);
    }
}