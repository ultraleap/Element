using System.Collections.Generic;
using Lexico;

#pragma warning disable 169, 8618

namespace Element.AST
{
    [WhitespaceSurrounded(ParserFlags = ParserFlags.IgnoreInTrace), MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class ListOf<T>
    {
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        // ReSharper disable once CollectionNeverUpdated.Global
        [
            field:
            SurroundBy("(", ")"),
            SeparatedBy(typeof(ListSeparator))
        ] public List<T> List { get; private set; }
        
        public override string ToString() => $"({string.Join(", ", List)})";
    }
    
    [WhitespaceSurrounded(ParserFlags = ParserFlags.IgnoreInTrace), MultiLine]
    internal struct ListSeparator
    {
        [Literal(",")] private Unnamed _;
        
        public override string ToString() => ", ";
    }
}