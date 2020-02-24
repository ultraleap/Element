using Lexico;

namespace Element.AST
{
    public struct Terminal
    {
        [Literal(";")] private Unnamed _;
        public override string ToString() => ";";
    }
}