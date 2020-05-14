using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    public class ListOf<T>
    {
#pragma warning disable 169, 8618
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        // ReSharper disable once CollectionNeverUpdated.Global
        [field: SurroundBy("(", ")"), SeparatedBy(typeof(ListSeparator))] public List<T> List { get; private set; }
#pragma warning restore 169, 8618

        public override string ToString() => $"({string.Join(", ", List)})";
    }
    
    [WhitespaceSurrounded, MultiLine]
    internal struct ListSeparator
    {
#pragma warning disable 169
        [Literal(",")] private Unnamed _;
#pragma warning restore 169
        public override string ToString() => ", ";
    }
}