using System;
using System.Collections.Generic;

namespace Element.AST
{
    public abstract class DeclaredScope : ScopeBase, IIdentifiable
    {
        protected abstract IEnumerable<DeclaredItem> ItemsToCacheOnValidate { get; }
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
            if (compilationContext.Input.SkipValidation) return true;
            
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

                if (Contains(item.Identifier))
                {
                    compilationContext.LogError(2, $"Cannot add duplicate identifier '{item.Identifier}'");
                    success = false;
                }
                else
                {
                    Add(item.Identifier, item);
                }
            }

            return success;
        }
    }
}