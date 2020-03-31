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
        public static IValue CompileFunction(this IScope callScope, object body, CompilationContext compilationContext)  =>
            body switch
            {
                // If a function has expression body we just compile the single expression using the call scope
                ExpressionBody exprBody => exprBody.Expression.ResolveExpression(callScope, compilationContext),
                // If a function has a scope body we need to find the return identifier
                IScope scopeBody => scopeBody[Parser.ReturnIdentifier, false, compilationContext] switch
                {
                    ICompilableFunction nullaryReturn when nullaryReturn.IsNullary() => nullaryReturn.Compile(scopeBody, compilationContext),
                    ICompilableFunction functionReturn => functionReturn,
                    null => compilationContext.LogError(7, $"'{Parser.ReturnIdentifier}' not found in function scope"),
                    var nyi => throw new NotImplementedException(nyi.ToString())
                },
                _ => CompilationErr.Instance
            };
        
        /// <summary>
        /// Enumerates all values in the given scope returning those matching the given filter and recursing into any child scopes that match the recurse predicate.
        /// </summary>
        public static List<IValue> EnumerateValues(this IEnumerable<IValue> scope, Predicate<IValue> filter)
        {
            var results = new List<IValue>();
            void RecurseValue(IValue value)
            {
                if (filter(value)) results.Add(value);
                
                if (value is Declaration declaration
                    && declaration.ChildScope != null)
                {
                    RecurseMultipleValues(declaration.ChildScope);
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