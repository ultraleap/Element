using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    public class Scope : DeclaredScope, IFunctionBody, IStructBody
    {
        [Literal("{")] private Unnamed _open;
        [Optional] private readonly List<Item> _items = new List<Item>();
        [Literal("}")] private Unnamed _close;

        protected override IEnumerable<Item> ItemsToCacheOnValidate => _items;
    }
}