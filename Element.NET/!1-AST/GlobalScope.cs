using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IDeclarationScope
    {
        IReadOnlyList<Declaration> Declarations { get; }
        Result<IScope> ResolveScope(IScope? parentScope, CompilationContext context);
    }

    public static class LexicalScopeExtensions
    {
        /// <summary>
        /// Enumerates top-level and nested declarations that match the filter, resolving them to IValues.
        /// Will not recurse into function scopes.
        /// </summary>
        public static Result<List<IValue>> EnumerateValues(this IDeclarationScope declarationScope, CompilationContext context, Predicate<Declaration> declarationFilter = null, Predicate<IValue> resolvedValueFilter = null)
        {
            var builder = new ResultBuilder<List<IValue>>(context, new List<IValue>());
            
            void Recurse(IScope? parentScope, IDeclarationScope declScope)
            {
                void ResolveScopeValues(IScope containingScope)
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
                        
                        builder.Append(childScope.ResolveScope(containingScope, context).Then(RecurseIntoChildScope));
                    }
                }

                builder.Append(declScope.ResolveScope(parentScope, context)
                                               .Then(ResolveScopeValues));
            }

            builder.Append(declarationScope.ResolveScope(null, context)
                                           .Then(scope => Recurse(scope, declarationScope)));
            return builder.ToResult();
        }
    }
    
    public sealed class GlobalScope : IScope, IDeclarationScope
    {
        private readonly Dictionary<string, SourceBlob> _sourceScopes = new Dictionary<string, SourceBlob>();
        private IReadOnlyList<Declaration>? _cachedList;
        public IReadOnlyList<Declaration> Declarations => _cachedList ??= _sourceScopes.Values.SelectMany(blob => blob).ToList();
        public Result<IScope> ResolveScope(IScope? parentScope, CompilationContext context) => this;

        public bool ContainsSource(string sourceName) => _sourceScopes.ContainsKey(sourceName);
        
        public Result<SourceBlob> GetSource(string sourceName, ITrace trace) =>
            _sourceScopes.TryGetValue(sourceName, out var found)
                ? new Result<SourceBlob>(found)
                : trace.Trace(MessageCode.ArgumentNotFound, $"No source named '{sourceName}'");

        public bool RemoveSource(string sourceName)
        {
            if (!_sourceScopes.Remove(sourceName)) return false;
            // Don't need to validate again, removing source cannot invalidate the global scope
            _cachedList = null;
            return true;
        }

        public Result AddSource(SourceInfo source, SourceContext sourceContext) =>
            ContainsSource(source.Name)
                ? sourceContext.Trace(MessageCode.DuplicateSourceFile, $"Duplicate source '{source.Name}'")
                : Parser.Parse<SourceBlob>(source, sourceContext, sourceContext.CompilationInput.NoParseTrace)
                        .Bind(blob =>
                        {
                            _sourceScopes[source.Name] = blob;
                            _cachedList = null;
                            var validateResult = Validate(new CompilationContext(sourceContext));
                            if (validateResult.IsError) RemoveSource(source.Name);
                            return validateResult;
                        });

        public Result<IValue> Index(Identifier id, CompilationContext context) =>
            Declarations.FirstOrDefault(d => d.Identifier == id)
                               ?.Resolve(this, context) // Top-level declarations can be resolved with the global scope since outer captures are impossible!
            ?? (Result<IValue>)context.Trace(MessageCode.IdentifierNotFound, $"'{id}' not found in global scope");

        public Result<IValue> Lookup(Identifier id, CompilationContext context) => Index(id, context); // Nowhere up to go from here!
        public IReadOnlyList<Identifier> Members => Declarations.Select(d => d.Identifier).ToList();

        public Result Validate(CompilationContext context)
        {
            var resultBuilder = new ResultBuilder(context);
            var idHashSet = new HashSet<Identifier>();
            foreach (var decl in Declarations)
            {
                decl.Identifier.Validate(resultBuilder, Array.Empty<Identifier>(), Array.Empty<Identifier>());
                decl.Validate(resultBuilder, context);
                if (!idHashSet.Add(decl.Identifier))
                {
                    resultBuilder.Append(MessageCode.MultipleDefinitions, $"Multiple definitions for '{decl.Identifier}' defined in global scope");
                }
            }

            return resultBuilder.ToResult();
        }
    }
}