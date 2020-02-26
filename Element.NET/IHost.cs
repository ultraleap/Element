using System.IO;

namespace Element
{
    public interface IHost
    {
        bool ParseFile(CompilationInput input, FileInfo file);
        (bool Success, float[] Result) Evaluate(CompilationInput input, string expression);
        (bool Success, string Result) Typeof(CompilationInput input, string expression);
    }
}