using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Scope : ScopeBase, IDeclared
    {
#pragma warning disable 649, 169, 8618
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [Location] public int IndexInSource { get; private set; }
        [SurroundBy("{", "}"), WhitespaceSurrounded, Optional] private List<Declaration>? _items;
#pragma warning restore 649, 169
        
        public Declaration Declarer { get; private set; }
#pragma warning restore 8618

        public override Result<IValue> this[Identifier id, bool recurse, CompilationContext context] =>
            Index(id)
                .ElseIf(recurse, () => Declarer.Parent[id, true, context]);

        protected override IList<(Identifier Identifier, IValue Value)> _source => _items.Select(item => (item.Identifier, (IValue)item)).ToList();

        public void Initialize(Declaration declarer, IIntrinsicCache? cache)
        {
            Declarer = declarer ?? throw new ArgumentNullException(nameof(declarer));
            foreach (var item in _items ?? Enumerable.Empty<Declaration>())
            {
                item.Initialize(Declarer.SourceInfo, this, cache);
            }
        }
    }
}