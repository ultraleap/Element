using System.IO;

namespace Element
{
    public interface IHost
    {
        bool ParseFile(CompilationInput input, FileInfo file);
        float[] Evaluate(CompilationInput input, string expression);
    }
}