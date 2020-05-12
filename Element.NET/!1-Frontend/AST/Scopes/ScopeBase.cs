using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;

namespace Element.AST
{
    public abstract class ScopeBase : IScope
    {
        public abstract IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] { get; }
        protected IValue IndexCache(int index) => (IValue)_cache[index];
        protected IValue? IndexCache(Identifier id) => Contains(id) ? (IValue)_cache[id] : null;
        private readonly OrderedDictionary _cache = new OrderedDictionary();
        protected bool Contains(Identifier id) => _cache.Contains(id);
        public int Count => _cache.Count;
        protected void Set(Identifier id, IValue scopeItem) => _cache[id] = scopeItem;
        protected void SetRange(IEnumerable<(Identifier, IValue)> values)
        {
            foreach (var (identifier, value) in values)
            {
                Set(identifier, value);
            }
        }

        public IEnumerator<IValue> GetEnumerator() => _cache.Values.Cast<IValue>().GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }
}