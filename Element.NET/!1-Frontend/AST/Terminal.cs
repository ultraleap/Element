using Lexico;

namespace Element.AST
{
    struct Terminal : IFunctionBody, IStructBody
    {
        [Literal(";")] private Unnamed _;
        public override string ToString() => ";";
    }
}