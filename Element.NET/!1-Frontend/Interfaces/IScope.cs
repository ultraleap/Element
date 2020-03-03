namespace Element.AST
{
    public interface IScope
    {
        IValue? this[Identifier id, CompilationContext compilationContext] { get; }
    }
}