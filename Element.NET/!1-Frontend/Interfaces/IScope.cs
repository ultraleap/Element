using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IIndexable
    {
        IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] { get; }
    }

    public interface IScope : IIndexable, IReadOnlyCollection<IValue> { }

    public static class ScopeExtensions
    {
        private class ClonedScope : ScopeBase, IDeclared
        {
            private readonly IScope _parent;

            public ClonedScope(Declaration declarer, IEnumerable<Declaration> items, IScope parent)
            {
                Declarer = declarer;
                _parent = parent;
                SetRange(items.Select(item => (item.Identifier, (IValue)item.Clone(this))));
            }

            public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
                IndexCache(id) ?? (recurse ? _parent[id, true, compilationContext] : null);

            public Declaration Declarer { get; }
        }

        public static IScope Clone(this IScope scope, Declaration declarer, IScope parent)=>
            new ClonedScope(declarer, scope.Cast<Declaration>(), parent);
        
        /// <summary>
        /// Enumerates all values in the given scope returning those matching the given filter.
        /// </summary>
        public static List<TValue> EnumerateValues<TValue>(this IScope scope, Predicate<TValue> filter) where TValue : IValue
        {
            var results = new List<TValue>();
            void RecurseValue(IValue value)
            {
                if (value is TValue tval && filter(tval)) results.Add(tval);
                
                if (value is Declaration decl && decl.Child != null)
                {
                    RecurseMultipleValues(decl.Child);
                }
            }

            void RecurseMultipleValues(IEnumerable<IValue> values)
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