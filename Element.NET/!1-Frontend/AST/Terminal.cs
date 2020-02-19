using Lexico;

namespace Element.AST
{
    internal struct Terminal : IFunctionBody, IStructBody
    {
        [Literal(";")] private Unnamed _;
        public override string ToString() => ";";
    }
}