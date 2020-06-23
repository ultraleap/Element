using System.Collections;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    /// <summary>
    /// The global scope, root of all other scopes
    /// </summary>
    [WhitespaceSurrounded, MultiLine, TopLevel]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class SourceScope : IEnumerable<Declaration>
    {
#pragma warning disable 649
        // ReSharper disable once CollectionNeverUpdated.Local
        [Optional] private List<Declaration>? _items;
#pragma warning restore 649
        
        public void InitializeItems(SourceInfo info, IScope parent, SourceContext sourceContext)
        {
            foreach (var item in this)
            {
                item.Initialize(info, parent, sourceContext);
            }
        }

        public IEnumerator<Declaration> GetEnumerator() => _items?.GetEnumerator() ?? Enumerable.Empty<Declaration>().GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }
}