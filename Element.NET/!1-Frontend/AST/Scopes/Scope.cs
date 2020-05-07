using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Scope : DeclaredScope, IDeclared
    {
#pragma warning disable 649, 169
        [SurroundBy("{", "}"), WhitespaceSurrounded, Optional] private List<Declaration>? _items;
#pragma warning restore 649, 169

        protected override IEnumerable<Declaration> ItemsToCacheOnValidate => _items ?? Enumerable.Empty<Declaration>();

        public Declaration Declarer { get; private set; }

        public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
            IndexCache(id) ?? (recurse ? Declarer.Parent[id, true, compilationContext] : null);

        public void Initialize(Declaration declarer)
        {
            Declarer = declarer ?? throw new ArgumentNullException(nameof(declarer));
            InitializeItems();
        }
    }
}