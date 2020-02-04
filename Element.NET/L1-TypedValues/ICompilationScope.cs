using Element.AST;

namespace Element
{
    public interface ICompilationScope
    {
        bool AddItem(string identifier, Item item, CompilationContext compilationContext);
        IValue Compile(string identifier, CompilationContext context);
    }
}