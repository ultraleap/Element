using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class GlobalScope : ScopeBase
    {
        private readonly Dictionary<string, SourceScope> _sourceScopes = new Dictionary<string, SourceScope>();
        private IList<(Identifier Identifier, IValue Value)>? _cachedList;

        public bool ContainsSource(string sourceName) => _sourceScopes.ContainsKey(sourceName);
        
        public Result<SourceScope> GetSource(string sourceName) =>
            _sourceScopes.TryGetValue(sourceName, out var found)
                ? new Result<SourceScope>(found)
                : new Result<SourceScope>(new CompilerMessage(MessageCode.ArgumentNotFound, $"No source named '{sourceName}'"));

        public Result AddSource(string sourceName, SourceScope sourceScope)
        {
            if (ContainsSource(sourceName)) return (MessageCode.DuplicateSourceFile, $"Duplicate source '{sourceName}'");
            _sourceScopes[sourceName] = sourceScope;
            _cachedList = null;
            return Result.Success;
        }

        public override Result<IValue> this[Identifier id, bool recurse, CompilationContext context] => Index(id);

        protected override IList<(Identifier Identifier, IValue Value)> _source => _cachedList ??= _sourceScopes.Values
                                                                                                                .SelectMany(scope => scope.Select(declaration => (declaration.Identifier, (IValue) declaration)))
                                                                                                                .ToList();
    }
}