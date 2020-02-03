namespace Element
{
    public interface ICompilationScope
    {
        bool AddParseMatch(string identifier, ParseMatch parseMatch, CompilationContext compilationContext);
        IValue Compile(string identifier, CompilationContext context);
    }
}