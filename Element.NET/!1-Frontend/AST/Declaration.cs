using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    public class Declaration
    {
        [field: Term] public Identifier Identifier { get; }
        [field: Optional] public PortList PortList { get; }
        [field: Optional] public Type ReturnType { get; }

        public override string ToString() => $"{Identifier}{PortList}";
    }
}