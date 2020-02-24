using System.Collections;
using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    /// <summary>
    /// The global scope, root of all other scopes
    /// </summary>
    [WhitespaceSurrounded, MultiLine, TopLevel]
    public class SourceScope : IEnumerable<DeclaredItem>
    {
        [Optional] private readonly List<DeclaredItem> _items = new List<DeclaredItem>();

        public IEnumerator<DeclaredItem> GetEnumerator() => _items.GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

        public override string ToString() => "SourceScope";
    }
}