using System.Collections.Generic;

namespace Element.AST
{
    public abstract class ScopeBase : IScope
    {
        private readonly Dictionary<Identifier, IScopeItem> _cache = new Dictionary<Identifier, IScopeItem>();
        public IScope? Parent { get; protected set; }
        public IScopeItem? this[Identifier id] => _cache.TryGetValue(id, out var value) ? value : null;
        protected bool Contains(Identifier id) => _cache.ContainsKey(id);
        protected void Add(Identifier id, IScopeItem scopeItem) => _cache[id] = scopeItem;
        protected void AddRange(IEnumerable<(Identifier, IValue)> values)
        {
            foreach (var (identifier, value) in values)
            {
                Add(identifier, value);
            }
        }
    }
}