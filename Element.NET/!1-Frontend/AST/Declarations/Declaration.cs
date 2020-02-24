using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Declaration
    {
        // ReSharper disable once UnassignedGetOnlyAutoProperty
        [field: Term] public Identifier Identifier { get; }
        // ReSharper disable once UnassignedGetOnlyAutoProperty
        [field: Optional] public PortList PortList { get; }
        // ReSharper disable once UnassignedGetOnlyAutoProperty
        [field: Optional] public Type Type { get; }

        public override string ToString() => $"{Identifier}{PortList}";
    }
}