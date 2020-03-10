using System.IO;

namespace Element
{
    /// <summary>
    /// Host where each command executes in a temporary compilation context and no compilation state is persisted between commands.
    /// </summary>
    public class AtomicHost : IHost
    {
        public bool ParseFile(CompilationInput compilationInput, FileInfo file) => new PersistentHost(compilationInput).ParseFile(compilationInput, file);
        public (bool Success, float[] Result) Evaluate(CompilationInput compilationInput, string expression) => new PersistentHost(compilationInput).Evaluate(compilationInput, expression);
        public (bool Success, string Result) Typeof(CompilationInput input, string expression) => new PersistentHost(input).Typeof(input, expression);
    }
}