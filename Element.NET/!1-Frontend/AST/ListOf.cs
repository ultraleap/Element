using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    public abstract class ListOf<T>
    {
#pragma warning disable 169
        [Literal("(")] private Unnamed _open;
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        // ReSharper disable once CollectionNeverUpdated.Global
        [field: SeparatedBy(typeof(ListSeparator))] public List<T> List { get; private set; }
        [Literal(")")] private Unnamed _close;
#pragma warning restore 169

        public override string ToString() => $"({string.Join(", ", List)})";
    }
    
    [WhitespaceSurrounded, MultiLine]
    internal struct ListSeparator
    {
        [Literal(",")] private Unnamed _;
        public override string ToString() => ", ";
    }
}