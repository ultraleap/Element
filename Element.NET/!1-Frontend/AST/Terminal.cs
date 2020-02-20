using Lexico;

namespace Element.AST
{
    public struct Terminal : IFunctionBody, IStructBody
    {
        [Literal(";")] private Unnamed _;
        public override string ToString() => ";";
    }
}