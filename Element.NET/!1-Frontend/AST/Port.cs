using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Port
    {
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public Identifier Identifier { get; private set; }
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Optional] public Type Type { get; private set; }

        public override string ToString() => $"{Identifier}{Type}";
    }
    
    // ReSharper disable once ClassNeverInstantiated.Global
    public class PortList : ListOf<Port> { } // CallExpression looks like a list due to using brackets
}