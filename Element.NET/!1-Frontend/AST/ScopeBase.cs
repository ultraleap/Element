using System;
using System.Collections.Generic;

namespace Element.AST
{
    public abstract class ScopeBase : IIndexable
    {
        public ScopeBase() {} // Parameterless constructor used by Lexico when constructing parse matches

        protected ScopeBase(IIndexable? parent)
        {
            Parent = parent;
        }

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

        public IIndexable? Parent { get; private set; }
            
        public bool ValidateScope(IIndexable parent, CompilationContext compilationContext, List<Identifier> identifierWhitelist = null)
        {
            var success = true;
            if (!(this is GlobalScope)) // Global scope has no parent!
            {
                Parent = parent ?? throw new ArgumentNullException(nameof(parent));
            }

            foreach (var item in ItemsToCacheOnValidate)
            {
                if (!compilationContext.ValidateIdentifier(item.Identifier, identifierWhitelist))
                {
                    success = false;
                }
                
                if (!item.Validate(this, compilationContext))
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