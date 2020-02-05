using Element.AST;

namespace Element
{
    public static class CompilationExtensions
    {
        public static IValue Compile(in AST.Expression expression, in CompilationContext compilationContext, in Indexer indexer)
        {
            return expression.LitOrId switch
            {
                Literal lit => lit,
                Identifier id => indexer(id, compilationContext) is var item
                    ? item switch
                    {
                        Function function => function.Call(null),
                        _ => CompilationErr.Instance
                    } : CompilationErr.Instance,
                _ => CompilationErr.Instance
            };
        }
    }
}