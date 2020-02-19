using System;
using System.Collections.Generic;

namespace Element.AST
{
    public abstract class ScopeBase : IScope
    {
        protected readonly Dictionary<Identifier, IValue> _valueCache = new Dictionary<Identifier, IValue>();
        public IScope? Parent { get; protected set; }
        public bool CanBeCached => true;
        public IValue? this[Identifier id]
        {
            get
            {
                if (_valueCache.TryGetValue(id, out var value)) return value;
                if (TryGetUncached(id, out value) && value.CanBeCached)
                {
                    _valueCache[id] = value;
                }

                return value;
            }
        }

        protected virtual bool TryGetUncached(Identifier identifier, out IValue value)
        {
            value = default;
            return false;
        }
    }

    public abstract class DeclaredScope : ScopeBase, IIdentifiable
    {
        private readonly Dictionary<Identifier, Item> _itemCache = new Dictionary<Identifier, Item>();
        protected abstract IEnumerable<Item> ItemsToCacheOnValidate { get; }
        public Identifier Identifier { get; private set; }

        public void Initialize(DeclaredScope parent, IIdentifiable declarer)
        {
            if (!(this is GlobalScope)) // Global scope has no parent or declarer!
            {
                Parent = parent ?? throw new ArgumentNullException(nameof(parent));
                Identifier = declarer?.Identifier ?? throw new ArgumentNullException(nameof(declarer));
            }

            foreach (var item in ItemsToCacheOnValidate)
            {
                item.Initialize(this);
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

        protected override bool TryGetUncached(Identifier identifier, out IValue value)
        {
            var success = _itemCache.TryGetValue(identifier, out var item);
            value = success switch
            {
                true => item switch
                {
                    IValue v => v,
                    _ => throw new InternalCompilerException($"{item} is not an IValue")
                },
                false => null // Didn't find so return null - this is not always an error! e.g. when looking through frames recursively
            };
            return success;
        }
    }

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
                _valueCache.Add(identifier, value);
            }
        }
    }
}