using System.Collections.Generic;

namespace Element.AST
{
    public abstract class ScopeBase : IIndexable
    {
        private readonly Dictionary<string, Item> _itemCache = new Dictionary<string, Item>();
        private readonly Dictionary<string, IValue> _valueCache = new Dictionary<string, IValue>();
        protected abstract IEnumerable<Item> ItemsToCacheOnValidate { get; }
            
        public bool Validate(CompilationContext compilationContext)
        {
            var success = true;
            foreach (var item in ItemsToCacheOnValidate)
            {
                if (!compilationContext.ValidateIdentifier(item.Identifier))
                {
                    success = false;
                    continue;
                }
                
                if (_itemCache.ContainsKey(item.Identifier))
                {
                    compilationContext.LogError(2, $"Cannot add duplicate identifier '{item.Identifier}'");
                    success = false;
                    continue;
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