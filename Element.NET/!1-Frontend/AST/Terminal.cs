using Lexico;

namespace Element.AST
{
    public struct Terminal
    {
#pragma warning disable 169
        [Literal(";")] private Unnamed _;
#pragma warning restore 169

        public override string ToString() => ";";
    }
}