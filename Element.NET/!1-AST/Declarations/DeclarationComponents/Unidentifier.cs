using Lexico;

namespace Element.AST
{
    // ReSharper disable once IdentifierTypo
    public struct Unidentifier
    {
#pragma warning disable 169
        [Literal("_")] private Unnamed _;
#pragma warning restore 169
    }
}