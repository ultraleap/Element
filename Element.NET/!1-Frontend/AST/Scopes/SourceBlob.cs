using System.Collections;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    /// <summary>
    /// Represents a top-level blob of Element source code
    /// </summary>
    [WhitespaceSurrounded, MultiLine, TopLevel]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class SourceBlob : IEnumerable<Declaration>
    {
#pragma warning disable 649
        // ReSharper disable once CollectionNeverUpdated.Local
        [Optional] private List<Declaration>? _items;
#pragma warning restore 649
        
        public void Initialize(in SourceInfo info, IScope parent, IIntrinsicCache? cache)
        {
            foreach (var item in this)
            {
                item.Initialize(in info, parent, cache);
            }
        }

        public IEnumerator<Declaration> GetEnumerator() => _items?.GetEnumerator() ?? Enumerable.Empty<Declaration>().GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }
}