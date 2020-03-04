using System.Collections.Generic;

namespace Element.AST
{
    public abstract class ScopeBase : IScope
    {
        public abstract IValue? this[Identifier id, bool recurse, CompilationContext context] { get; }
        protected IValue? IndexCache(Identifier id) => _cache.TryGetValue(id, out var value) ? value : null;
        private readonly Dictionary<Identifier, IValue> _cache = new Dictionary<Identifier, IValue>();
        protected bool Contains(Identifier id) => _cache.ContainsKey(id);
        protected void Set(Identifier id, IValue scopeItem) => _cache[id] = scopeItem;
        protected void SetRange(IEnumerable<(Identifier, IValue)> values)
        {
            foreach (var (identifier, value) in values)
            {
                Set(identifier, value);
            }
        }
    }
}