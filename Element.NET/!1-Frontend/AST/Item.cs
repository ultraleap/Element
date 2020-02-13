using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    public abstract class Item
    {
        public abstract Identifier Identifier { get; }
        public abstract bool Validate(CompilationContext compilationContext);
    }
}