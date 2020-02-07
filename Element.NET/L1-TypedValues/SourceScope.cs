using System.Collections;
using System.Collections.Generic;
using Element.AST;
using Lexico;

namespace Element
{
    /// <summary>
    /// The global scope, root of all other scopes
    /// </summary>
    [WhitespaceSurrounded, EOFAfter]
    public class SourceScope : IEnumerable<Item>, IIndexable
    {
	    [Optional] private readonly List<Item> _items = new List<Item>();

	    private readonly Dictionary<string, Item> _uncompiledCache = new Dictionary<string, Item>();
        private readonly Dictionary<string, IValue> _compiledCache = new Dictionary<string, IValue>();

        public IEnumerator<Item> GetEnumerator() => _items.GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

        public override string ToString() => "<global>";

        public bool Validate(CompilationContext compilationContext) => compilationContext.ValidateScope(_items, _uncompiledCache);

        public Item? this[Identifier id, CompilationContext compilationContext] => compilationContext.Index(id, _items, _uncompiledCache);
    }
}