using System;
using System.Collections.Generic;

namespace Element.AST
{
    public interface IDeclarationScope
    {
        IReadOnlyList<Declaration> Declarations { get; }
        Result<ResolvedBlock> ResolveBlock(IScope? parentScope, Context context);
    }
    
    public static class DeclarationScopeExtensions
    {
        /// <summary>
        /// Enumerates top-level and nested declarations that match the filter, resolving them to IValues.
        /// Will not recurse into function scopes.
        /// </summary>
        public static Result<List<IValue>> EnumerateValues(this IDeclarationScope declarationScope, Context context, Predicate<Declaration> declarationFilter = null, Predicate<IValue> resolvedValueFilter = null)
        {
            var builder = new ResultBuilder<List<IValue>>(context, new List<IValue>());
            
            void Recurse(IScope? parentScope, IDeclarationScope declScope)
            {
                builder.Append(declScope.ResolveBlock(parentScope, context)
                                        .Then(ResolveBlockValues));
                
                void ResolveBlockValues(IScope containingScope)
                {
                    foreach (var decl in declScope.Declarations)
                    {
                        if (declarationFilter?.Invoke(decl) ?? true)
                        {
                            void AddResolvedValueToResults(IValue v)
                            {
                                if (resolvedValueFilter?.Invoke(v) ?? true) builder.Result.Add(v);
                            }

                            builder.Append(decl.Resolve(containingScope, context).Then(AddResolvedValueToResults));
                        }

                        if (!(decl.Body is IDeclarationScope childScope)) continue;
                        
                        void RecurseIntoChildScope(IScope resolvedBlockScope) => Recurse(resolvedBlockScope, childScope);
                        
                        builder.Append(childScope.ResolveBlock(containingScope, context).Then(RecurseIntoChildScope));
                    }
                }
            }

            builder.Append(declarationScope.ResolveBlock(null, context)
                                           .Then(scope => Recurse(scope, declarationScope)));
            return builder.ToResult();
        }
    }
}