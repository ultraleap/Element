using System.Collections;
using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    /// <summary>
    /// The global scope, root of all other scopes
    /// </summary>
    [WhitespaceSurrounded, MultiLine, TopLevel]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class SourceScope : IEnumerable<DeclaredItem>
    {
#pragma warning disable 649
        // ReSharper disable once CollectionNeverUpdated.Local
        [Optional] private List<DeclaredItem>? _items;
#pragma warning restore 649

        public IEnumerator<DeclaredItem> GetEnumerator() => _items?.GetEnumerator() ?? EmptyEnumerator<DeclaredItem>.Instance;
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

        public override string ToString() => "SourceScope";
    }
}