using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Type
    {
#pragma warning disable 169
        [Literal(":"), WhitespaceSurrounded, MultiLine] private Unnamed _;
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public Expression Expression { get; private set; }
#pragma warning restore 169

        public override string ToString() => $":{Expression}";
    }
}