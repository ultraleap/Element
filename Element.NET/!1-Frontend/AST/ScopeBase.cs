using System.Collections.Generic;

namespace Element.AST
{
    public abstract class ScopeBase : IIndexable
    {
        private readonly Dictionary<Identifier, Item> _itemCache = new Dictionary<Identifier, Item>();
        private readonly Dictionary<Identifier, IValue> _valueCache = new Dictionary<Identifier, IValue>();
        protected abstract IEnumerable<Item> ItemsToCacheOnValidate { get; }

        protected void AddRangeToCache(IEnumerable<(Identifier, IValue)> values)
        {
            foreach (var (identifier, value) in values)
            {
                _valueCache.Add(identifier, value);
            }
        }
            
        public bool ValidateScope(CompilationContext compilationContext, List<Identifier> identifierWhitelist = null)
        {
            var success = true;
            foreach (var item in ItemsToCacheOnValidate)
            {
                if (!compilationContext.ValidateIdentifier(item.Identifier, identifierWhitelist))
                {
                    success = false;
                }
                
                if (!item.Validate(compilationContext))
                {
                    success = false;
                }

                if (_itemCache.ContainsKey(item.Identifier))
                {
                    compilationContext.LogError(2, $"Cannot add duplicate identifier '{item.Identifier}'");
                    success = false;
                }

                _itemCache[item.Identifier] = item;
            }

            return success;
        }

        public IValue? this[Identifier id]
        {
            get
            {
                if (_valueCache.TryGetValue(id, out var value)) return value;
                value = _itemCache.TryGetValue(id, out var item) switch
                {
                    true => item switch
                    {
                        IValue v => v,
                        _ => throw new InternalCompilerException($"{item} is not an IValue")
                    },
                    false => null // Didn't find so return null - this is not always an error! e.g. when looking through frames recursively
                };
                if (value != null && value.CanBeCached)
                {
                    _valueCache[id] = value;
                }

                return value;
            }
        }

        public bool CanBeCached => true;
    }
}