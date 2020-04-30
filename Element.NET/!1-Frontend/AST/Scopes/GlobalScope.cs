using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Element.AST
{
    public sealed class GlobalScope : DeclaredScope
    {
        private readonly Dictionary<string, SourceScope> _sourceScopes = new Dictionary<string, SourceScope>();

        public SourceScope this[FileInfo file]
        {
            get => _sourceScopes.TryGetValue(file.FullName, out var found) ? found : null;
            set => _sourceScopes[file.FullName] = value;
        }

        public override IValue? this[Identifier id, bool recurse, CompilationContext context] => IndexCache(id);

        protected override IEnumerable<Declaration> ItemsToCacheOnValidate => _sourceScopes.Values.SelectMany(s => s);
    }
}