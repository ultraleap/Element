using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    /// <summary>
    /// A scope of declarations which can be resolved as a block.
    /// </summary>
    public interface IDeclarationScope
    {
        IReadOnlyList<Declaration> Declarations { get; }
        Result<ResolvedBlock> ResolveBlock(IScope? parentScope, Context context, Func<IValue>? valueProducedFrom = null);
    }

    /// <summary>
    /// Value retaining it's full path in source and identifiers within, including it's own identifier.
    /// </summary>
    public class ValueWithLocation
    {
        public ValueWithLocation(Identifier[] identifiersInPath, IValue value)
        {
            Identifier = identifiersInPath.Last();
            IdentifiersInPath = identifiersInPath;
            FullPath = string.Join(".", IdentifiersInPath);
            Value = value;
        }

        public Identifier Identifier { get; }
        public Identifier[] IdentifiersInPath { get; }
        public string FullPath { get; }
        public IValue Value { get; }
    }
    
    public static class DeclarationScopeExtensions
    {
        /// <summary>
        /// Enumerates top-level and nested declarations that match the filter, resolving them to IValues.
        /// Will not recurse into function scopes.
        /// </summary>
        public static Result<List<ValueWithLocation>> EnumerateValues(this IDeclarationScope declarationScope, Context context, Predicate<Declaration>? declarationFilter = null, Predicate<ValueWithLocation>? resolvedValueFilter = null)
        {
            var builder = new ResultBuilder<List<ValueWithLocation>>(context, new List<ValueWithLocation>());
            var idStack = new Stack<Identifier>();

            void Recurse(IScope? parentScope, IDeclarationScope declScope)
            {
                builder.Append(declScope.ResolveBlock(parentScope, context)
                                        .Do(ResolveBlockValues));
                
                void ResolveBlockValues(IScope containingScope)
                {
                    foreach (var decl in declScope.Declarations)
                    {
                        idStack.Push(decl.Identifier);
                        if (declarationFilter?.Invoke(decl) ?? true)
                        {
                            void AddResolvedValueToResults(IValue v)
                            {
                                var resolvedValue = new ValueWithLocation(idStack.Reverse().ToArray(), v);
                                if (resolvedValueFilter?.Invoke(resolvedValue) ?? true) builder.Result.Add(resolvedValue);
                            }

                            builder.Append(decl.Resolve(containingScope, context).Do(AddResolvedValueToResults));
                        }

                        if (decl.Body is IDeclarationScope childScope)
                        {
                            void RecurseIntoChildScope(IScope resolvedBlockScope) => Recurse(resolvedBlockScope, childScope);

                            builder.Append(childScope.ResolveBlock(containingScope, context).Do(RecurseIntoChildScope));
                        }

                        idStack.Pop();
                    }
                }
            }

            builder.Append(declarationScope.ResolveBlock(null, context)
                                           .Do(scope => Recurse(scope, declarationScope)));
            return builder.ToResult();
        }
    }
}