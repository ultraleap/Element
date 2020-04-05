using System.Collections.Generic;

namespace Element.AST
{
    public abstract class DeclaredScope : ScopeBase<Declaration>
    {
        protected abstract IEnumerable<Declaration> ItemsToCacheOnValidate { get; }

        public void InitializeItems()
        {
            foreach (var item in ItemsToCacheOnValidate)
            {
                item.Initialize(this);
            }
        }

        public bool ValidateScope(SourceContext sourceContext, Identifier[] identifierBlacklist = null, Identifier[] identifierWhitelist = null)
        {
            if (sourceContext.SkipValidation) return true;
            
            var success = true;

            foreach (var item in ItemsToCacheOnValidate)
            {
                if (item.HasBeenValidated) continue;

                if (Contains(item.Identifier))
                {
                    sourceContext.LogError(2, $"Cannot add duplicate identifier '{item.Identifier}'");
                    success = false;
                }
                else
                {
                    Set(item.Identifier, item);
                }

                if (!sourceContext.ValidateIdentifier(item.Identifier, identifierBlacklist, identifierWhitelist))
                {
                    success = false;
                }

                if (!item.Validate(sourceContext))
                {
                    success = false;
                }

                item.HasBeenValidated = true;
            }

            return success;
        }
    }
}