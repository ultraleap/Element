using System.Collections.Generic;

namespace Element.AST
{
    public abstract class ScopeBase : IScope
    {
        protected readonly Dictionary<Identifier, IScopeItem> _cache = new Dictionary<Identifier, IScopeItem>();
        public IScope? Parent { get; protected set; }
        public IScopeItem? this[Identifier id] => _cache.TryGetValue(id, out var value) ? value : null;
    }
}