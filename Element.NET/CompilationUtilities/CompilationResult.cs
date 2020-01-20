namespace Element
{
    public class CompilationResult<TResult>
    {
        public CompilationResult(TResult result, CompilationContext context)
        {
            Result = result;
            Context = context;
        }
        
        public TResult Result { get; }
        public CompilationContext Context { get; }

        public static implicit operator TResult(CompilationResult<TResult> result) => result.Result;
        public static implicit operator CompilationContext(CompilationResult<TResult> result) => result.Context;
    }
}