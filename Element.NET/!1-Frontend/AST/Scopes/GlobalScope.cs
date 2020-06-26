using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class GlobalScope : ScopeBase
    {
        private readonly Dictionary<string, SourceBlob> _sourceScopes = new Dictionary<string, SourceBlob>();
        private IList<(Identifier Identifier, IValue Value)>? _cachedList;

        public bool ContainsSource(string sourceName) => _sourceScopes.ContainsKey(sourceName);
        
        public Result<SourceBlob> GetSource(string sourceName, ITrace trace) =>
            _sourceScopes.TryGetValue(sourceName, out var found)
                ? new Result<SourceBlob>(found)
                : trace.Trace(MessageCode.ArgumentNotFound, $"No source named '{sourceName}'");

        public Result AddSource(string sourceName, SourceBlob sourceBlob, ITrace trace)
        {
            if (ContainsSource(sourceName)) return trace.Trace(MessageCode.DuplicateSourceFile, $"Duplicate source '{sourceName}'");
            _sourceScopes[sourceName] = sourceBlob;
            _cachedList = null;
            return Result.Success;
        }

        public override Result<IValue> this[Identifier id, bool recurse, CompilationContext context] => Index(id, context);

        protected override IList<(Identifier Identifier, IValue Value)> _source => _cachedList ??= _sourceScopes.Values
                                                                                                                .SelectMany(scope => scope.Select(declaration => (declaration.Identifier, (IValue) declaration)))
                                                                                                                .ToList();
    }
}