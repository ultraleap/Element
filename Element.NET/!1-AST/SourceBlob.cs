using System.Collections;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    /// <summary>
    /// Represents a top-level blob of Element source code.
    /// </summary>
    [WhitespaceSurrounded, MultiLine, TopLevel]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class SourceBlob : AstNode, IEnumerable<Declaration>
    {
#pragma warning disable 649
        // ReSharper disable once CollectionNeverUpdated.Local
        [Optional] private List<Declaration>? _items;
#pragma warning restore 649
        
        public IEnumerator<Declaration> GetEnumerator() => _items?.GetEnumerator() ?? Enumerable.Empty<Declaration>().GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

        protected override void ValidateImpl(ResultBuilder resultBuilder, Context context)
        {
            foreach (var decl in _items ?? Enumerable.Empty<Declaration>())
            {
                decl.Validate(resultBuilder, context);
            }
        }
    }
}