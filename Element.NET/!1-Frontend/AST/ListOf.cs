using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    public abstract class ListOf<T>
    {
#pragma warning disable 169
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        // ReSharper disable once CollectionNeverUpdated.Global
        [field: SurroundBy("(", ")"), SeparatedBy(typeof(ListSeparator))] public List<T> List { get; private set; }
#pragma warning restore 169

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