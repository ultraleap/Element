using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Scope : DeclaredScope, IFunctionBody, IStructBody
    {
        [Literal("{")] private Unnamed _open;
        [Optional] private readonly List<DeclaredItem> _items = new List<DeclaredItem>();
        [Literal("}")] private Unnamed _close;

        protected override IEnumerable<DeclaredItem> ItemsToCacheOnValidate => _items;
    }
}