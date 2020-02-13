using System.Collections;
using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    /// <summary>
    /// The global scope, root of all other scopes
    /// </summary>
    [WhitespaceSurrounded, EOFAfter]
    public class SourceScope : IEnumerable<Item>
    {
        [Optional] private readonly List<Item> _items = new List<Item>();

        public IEnumerator<Item> GetEnumerator() => _items.GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

        public override string ToString() => "SourceScope";
    }
}