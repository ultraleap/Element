using System.Collections.Generic;

namespace Element.AST
{
    public abstract class TransientScope : ScopeBase
    {
        protected TransientScope(IScope? parent = null)
        {
            Parent = parent;
        }

        protected void AddRangeToCache(IEnumerable<(Identifier, IValue)> values)
        {
            foreach (var (identifier, value) in values)
            {
                _cache.Add(identifier, value);
            }
        }
    }
}