namespace Element
{
    public interface ICompilationScope
    {
        IValue Compile(string identifier, CompilationContext context);
    }
}