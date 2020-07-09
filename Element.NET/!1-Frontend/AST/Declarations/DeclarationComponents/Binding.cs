using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    public class Binding : ExpressionBody
    {
#pragma warning disable 169
        [Term] private Terminal _terminal;
#pragma warning restore 169
    }
}