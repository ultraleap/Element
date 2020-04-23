using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;

namespace Element.AST
{
    public abstract class ScopeBase<TScopeValue> : IScope, IEnumerable<TScopeValue> where TScopeValue : class, IValue
    {
        public abstract IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] { get; }
        protected TScopeValue? IndexCache(Identifier id) => Contains(id) ? (TScopeValue)_cache[id] : null;
        private readonly OrderedDictionary _cache = new OrderedDictionary();
        protected bool Contains(Identifier id) => _cache.Contains(id);
        protected void Set(Identifier id, TScopeValue scopeItem) => _cache[id] = scopeItem;
        protected void SetRange(IEnumerable<(Identifier, TScopeValue)> values)
        {
            foreach (var (identifier, value) in values)
            {
                Set(identifier, value);
            }
        }

        public IEnumerator<TScopeValue> GetEnumerator() => _cache.Values.Cast<TScopeValue>().GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }
}