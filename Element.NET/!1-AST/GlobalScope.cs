using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class GlobalScope : IScope, IDeclarationScope
    {
        private readonly Dictionary<string, SourceBlob> _sourceScopes = new Dictionary<string, SourceBlob>();
        private IReadOnlyList<Declaration>? _cachedList;
        private readonly Dictionary<Identifier, Result<IValue>> _resolvedValueCache = new Dictionary<Identifier, Result<IValue>>();
        public IReadOnlyList<Declaration> Declarations => _cachedList ??= _sourceScopes.Values.SelectMany(blob => blob).ToList();
        public Result<ResolvedBlock> ResolveBlock(IScope? parentScope, Context context) =>
            new Result<ResolvedBlock>(
                new ResolvedBlock(Declarations.Select(d => d.Identifier).ToArray(),
                                  Enumerable.Empty<(Identifier Identifier, IValue Value)>(),
                    (resolvedBlock, identifier, indexingContext) => Lookup(identifier, indexingContext), null),
                context.Trace(MessageLevel.Information, $"Resolved global scope as block <{Declarations.Count} top level declarations>"));

        public bool ContainsSource(string sourceName) => _sourceScopes.ContainsKey(sourceName);
        
        public Result<SourceBlob> GetSource(string sourceName, Context context) =>
            _sourceScopes.TryGetValue(sourceName, out var found)
                ? new Result<SourceBlob>(found)
                : context.Trace(MessageCode.ArgumentNotFound, $"No source named '{sourceName}'");

        private bool RemoveSource(string sourceName)
        {
            if (!_sourceScopes.Remove(sourceName)) return false;
            // Don't need to validate again, removing source cannot invalidate the global scope
            _cachedList = null;
            _resolvedValueCache.Clear(); // TODO: Only remove identifiers from the removed sourced
            return true;
        }

        public Result AddSource(SourceInfo source, Context context) =>
            ContainsSource(source.Name)
                ? context.Trace(MessageCode.DuplicateSourceFile, $"Duplicate source '{source.Name}'")
                : Parser.Parse<SourceBlob>(source, context, context.CompilerOptions.NoParseTrace)
                        .Bind(blob =>
                        {
                            _sourceScopes[source.Name] = blob;
                            _cachedList = null;
                            var validateResult = Validate(context);
                            if (validateResult.IsError) RemoveSource(source.Name);
                            return validateResult;
                        });

        public Result<IValue> Lookup(Identifier id, Context context) =>
            _resolvedValueCache.TryGetValue(id, out var result)
                ? result
                : _resolvedValueCache[id] = Declarations.FirstOrDefault(d => d.Identifier.Equals(id))
                                                        ?.Resolve(this, context) // Top-level declarations can be resolved with the global scope since outer captures are impossible!
                                            ?? (Result<IValue>) context.Trace(MessageCode.IdentifierNotFound, $"'{id}' not found in global scope");

        public Result Validate(Context context)
        {
            var builder = new ResultBuilder(context);
            builder.Append(MessageLevel.Information, "Validating global scope...");
            var idHashSet = new HashSet<Identifier>();
            foreach (var decl in Declarations)
            {
                decl.Identifier.Validate(builder, Array.Empty<Identifier>(), Array.Empty<Identifier>());
                decl.Validate(builder, context);
                if (!idHashSet.Add(decl.Identifier))
                {
                    builder.Append(MessageCode.MultipleDefinitions, $"Multiple definitions for '{decl.Identifier}' defined in global scope");
                }
            }
            
            builder.Append(MessageLevel.Information, "Finished validating global scope");

            return builder.ToResult();
        }
    }
}