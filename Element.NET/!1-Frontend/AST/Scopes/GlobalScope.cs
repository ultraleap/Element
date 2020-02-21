using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Element.AST
{
    public sealed class GlobalScope : DeclaredScope
    {
        private readonly Dictionary<FileInfo, SourceScope> _sourceScopes = new Dictionary<FileInfo, SourceScope>();

        public SourceScope this[FileInfo file]
        {
            get => _sourceScopes[file];
            set => _sourceScopes[file] = value;
        }

        protected override IEnumerable<DeclaredItem> ItemsToCacheOnValidate => _sourceScopes.Values.SelectMany(s => s);
    }
}