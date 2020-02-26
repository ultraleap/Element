using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Scope : DeclaredScope
    {
        [Literal("{")] private Unnamed _open;
        [Optional] private readonly List<DeclaredItem> _items = new List<DeclaredItem>();
        [Literal("}")] private Unnamed _close;

        protected override IEnumerable<DeclaredItem> ItemsToCacheOnValidate => _items;
        public override string Location => Declarer.Location;
    }
}