using System.Collections.Generic;

namespace Element.AST
{
    public class Namespace : Value
    {
        private readonly ResolvedBlock _resolvedBlock;
        public Namespace(ResolvedBlock resolvedBlock, Identifier identifier, string? location = null) : base(location)
        {
            _resolvedBlock = resolvedBlock;
            Identifier = identifier;
        }
        
        public override Identifier? Identifier { get; }
        public override IReadOnlyList<Identifier> Members => _resolvedBlock.Members;
        public override Result<IValue> Index(Identifier id, CompilationContext context) => _resolvedBlock.Index(id, context);
    }
}