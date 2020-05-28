using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class GlobalScope : DeclaredScope
    {
        private readonly Dictionary<string, SourceScope> _sourceScopes = new Dictionary<string, SourceScope>();

        public SourceScope? this[string source]
        {
            get => _sourceScopes.TryGetValue(source, out var found) ? found : null;
            set { if (value != null) _sourceScopes[source] = value; }
        }

        public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] => IndexCache(id);

        protected override IEnumerable<Declaration> ItemsToCacheOnValidate => _sourceScopes.Values
                                                                                           .Cast<IEnumerable<Declaration>>()
                                                                                           .SelectMany(s => s);
    }
}