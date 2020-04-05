using System.Collections;
using System.Collections.Generic;

namespace Element.AST
{
    public abstract class ScopeBase<TScopeValue> : IScope, IEnumerable<TScopeValue> where TScopeValue : class, IValue
    {
        public abstract IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] { get; }
        protected TScopeValue? IndexCache(Identifier id) => _cache.TryGetValue(id, out var value) ? value : null;
        private readonly Dictionary<Identifier, TScopeValue> _cache = new Dictionary<Identifier, TScopeValue>();
        protected bool Contains(Identifier id) => _cache.ContainsKey(id);
        protected void Set(Identifier id, TScopeValue scopeItem) => _cache[id] = scopeItem;
        protected void SetRange(IEnumerable<(Identifier, TScopeValue)> values)
        {
            foreach (var (identifier, value) in values)
            {
                Set(identifier, value);
            }
        }

        public IEnumerator<TScopeValue> GetEnumerator() => _cache.Values.GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }
}