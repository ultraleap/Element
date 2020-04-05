using System;
using System.Collections.Generic;

namespace Element.AST
{
    public interface IScope
    {
        IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] { get; }
    }

    public static class ScopeExtensions
    {
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