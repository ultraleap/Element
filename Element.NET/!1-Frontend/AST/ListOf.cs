using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    public abstract class ListOf<T>
    {
        [Literal("(")] private Unnamed _open;
        [field: SeparatedBy(typeof(ListSeparator))] public List<T> List { get; }
        [Literal(")")] private Unnamed _close;

        public override string ToString() => $"({string.Join(", ", List)})";
    }
    
    [WhitespaceSurrounded, MultiLine]
    struct ListSeparator
    {
        [Literal(",")] private Unnamed _;
        public override string ToString() => ", ";
    }
}