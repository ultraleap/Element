using System.IO;

namespace Element
{
    public interface IHost
    {
        bool ParseFile(in CompilationInput input, FileInfo file);
        float[] Evaluate(in CompilationInput input, string expression);
    }
}