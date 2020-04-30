using Lexico;

namespace Element.AST
{
    public struct Unidentifier
    {
#pragma warning disable 169
        [Literal("_")] private Unnamed _;
#pragma warning restore 169

        public override string ToString() => "_";
    }
}