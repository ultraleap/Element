using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IScope
    {
        IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] { get; }
    }

    public static class ScopeExtensions
    {
        private class ClonedScope : ScopeBase<Declaration>, IDeclared
        {
            private readonly IScope _parentScope;

            public ClonedScope(Declaration declarer, IEnumerable<Declaration> items, IScope parentScope)
            {
                Declarer = declarer;
                _parentScope = parentScope;
                SetRange(items.Select(item => (item.Identifier, item.Clone(this))));
            }

            public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
                IndexCache(id) ?? (recurse ? _parentScope[id, true, compilationContext] : null);

            public Declaration Declarer { get; }
        }

        public static IScope Clone(this IScope scope, IScope parent) => new ClonedScope((scope as IDeclared)?.Declarer, scope as IEnumerable<Declaration>, parent);
        
        /// <summary>
        /// Enumerates all values in the given scope returning those matching the given filter.
        /// </summary>
        public static List<Declaration> EnumerateDeclarations(this IEnumerable<Declaration> scope, Predicate<Declaration> filter)
        {
            var results = new List<Declaration>();
            void RecurseValue(Declaration declaration)
            {
                if (filter(declaration)) results.Add(declaration);
                
                if (declaration.ChildScope != null)
                {
                    RecurseMultipleValues(declaration.ChildScope);
                }
            }

            void RecurseMultipleValues(IEnumerable<Declaration> values)
            {
                foreach (var v in values)
                {
                    RecurseValue(v);
                }
            }

            RecurseMultipleValues(scope);
            return results;
        }
    }
}