namespace Element.AST
{
    public interface IScope : IValue
    {
        IValue? this[Identifier id, CompilationContext compilationContext] { get; }
    }
}