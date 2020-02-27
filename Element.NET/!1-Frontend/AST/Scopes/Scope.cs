using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Scope : DeclaredScope
    {
#pragma warning disable 649, 169
        [Literal("{")] private Unnamed _open;
        // ReSharper disable once CollectionNeverUpdated.Local
        [Optional] private List<DeclaredItem>? _items;
        [Literal("}")] private Unnamed _close;
#pragma warning restore 649, 169

        protected override IEnumerable<DeclaredItem> ItemsToCacheOnValidate => _items ?? Enumerable.Empty<DeclaredItem>();
        public override string Location => Declarer.Location;
    }
}