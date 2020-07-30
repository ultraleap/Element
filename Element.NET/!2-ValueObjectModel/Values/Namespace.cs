using System;
using System.Collections.Generic;

namespace Element.AST
{
    public class Namespace : Value
    {
        private readonly ResolvedBlock _resolvedBlock;

        public Namespace(ResolvedBlock resolvedBlock) => _resolvedBlock = resolvedBlock;
        
        public override IReadOnlyList<Identifier> Members => _resolvedBlock.Members;
        public override Result<IValue> Index(Identifier id, CompilationContext context) => _resolvedBlock.Index(id, context);
        public override void Serialize(ResultBuilder<List<Element.Expression>> resultBuilder, CompilationContext context) => _resolvedBlock.Serialize(resultBuilder, context);
        public override Result<IValue> Deserialize(Func<Element.Expression> nextValue, CompilationContext context) =>
            _resolvedBlock.Deserialize(nextValue, context).Cast<ResolvedBlock>(context).Map(block => (IValue) new Namespace(block));
    }
}