using System.Collections.Generic;

namespace Element.AST
{
    public abstract class DeclaredScope : ScopeBase
    {
        protected abstract IEnumerable<Declaration> ItemsToCacheOnValidate { get; }

        public void InitializeItems()
        {
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
                if (Contains(item.Identifier))
                {
                    compilationContext.LogError(2, $"Cannot add duplicate identifier '{item.Identifier}'");
                    success = false;
                }
                else
                {
                    Set(item.Identifier, item);
                }

                if (!compilationContext.ValidateIdentifier(item.Identifier, identifierWhitelist))
                {
                    success = false;
                }

                if (!item.Validate(compilationContext))
                {
                    success = false;
                }
            }

            return success;
        }
    }
}