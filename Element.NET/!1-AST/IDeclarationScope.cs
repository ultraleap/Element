using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IDeclarationScope
    {
        IReadOnlyList<Declaration> Declarations { get; }
        Result<ResolvedBlock> ResolveBlock(IScope? parentScope, Context context);
    }

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
        public static Result<List<ValueWithLocation>> EnumerateValues(this IDeclarationScope declarationScope, Context context, Predicate<Declaration> declarationFilter = null, Predicate<IValue> resolvedValueFilter = null)
        {
            var builder = new ResultBuilder<List<ValueWithLocation>>(context, new List<ValueWithLocation>());
            var idStack = new Stack<Identifier>();

            void Recurse(IScope? parentScope, IDeclarationScope declScope)
            {
                builder.Append(declScope.ResolveBlock(parentScope, context)
                                        .Then(ResolveBlockValues));
                
                void ResolveBlockValues(IScope containingScope)
                {
                    foreach (var decl in declScope.Declarations)
                    {
                        idStack.Push(decl.Identifier);
                        if (declarationFilter?.Invoke(decl) ?? true)
                        {
                            void AddResolvedValueToResults(IValue v)
                            {
                                if (resolvedValueFilter?.Invoke(v) ?? true) builder.Result.Add(new ValueWithLocation(idStack.Reverse().ToArray(), v));
                            }

                            builder.Append(decl.Resolve(containingScope, context).Then(AddResolvedValueToResults));
                        }

                        if (decl.Body is IDeclarationScope childScope)
                        {
                            void RecurseIntoChildScope(IScope resolvedBlockScope) => Recurse(resolvedBlockScope, childScope);

                            builder.Append(childScope.ResolveBlock(containingScope, context).Then(RecurseIntoChildScope));
                        }

                        idStack.Pop();
                    }
                }
            }

            builder.Append(declarationScope.ResolveBlock(null, context)
                                           .Then(scope => Recurse(scope, declarationScope)));
            return builder.ToResult();
        }
    }
}