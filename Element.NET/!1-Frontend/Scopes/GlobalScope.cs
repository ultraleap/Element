using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class GlobalScope : IDeclarationScope
    {
        private readonly Dictionary<string, SourceBlob> _sourceScopes = new Dictionary<string, SourceBlob>();
        private IReadOnlyList<Declaration>? _cachedList;
        private IReadOnlyList<Declaration> _globalDeclarations => _cachedList ??= _sourceScopes.Values.SelectMany(blob => blob).ToList();
        
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
            _globalDeclarations.FirstOrDefault(d => d.Identifier == id)
                               ?.Resolve(this, context) // Top-level declarations can be resolved with the global scope since outer captures are impossible!
            ?? (Result<IValue>)context.Trace(MessageCode.IdentifierNotFound, $"'{id}' not found in global scope");

        public Result<IValue> Lookup(Identifier id, CompilationContext context) => Index(id, context); // Nowhere up to go from here!
        public IReadOnlyList<Identifier> Members => _globalDeclarations.Select(d => d.Identifier).ToList();

        public Result Validate(CompilationContext context)
        {
            var resultBuilder = new ResultBuilder(context);
            var idHashSet = new HashSet<Identifier>();
            foreach (var decl in _globalDeclarations)
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

        public IEnumerator<Declaration> GetEnumerator() => _globalDeclarations.GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }
}