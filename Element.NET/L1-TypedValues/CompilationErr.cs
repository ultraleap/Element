using Element.AST;

namespace Element
{
    /// <summary>
    /// A value that results from a failure during compilation. It will be accepted everywhere and generate no further
    /// errors, returning itself from each operation (the error is non-recoverable).
    /// </summary>
    public sealed class CompilationErr : IValue
    {
        public static IValue Instance { get; } = new CompilationErr();
        private CompilationErr() { }
        public override string ToString() => "<error>";
    }
}