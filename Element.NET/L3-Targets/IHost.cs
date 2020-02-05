using System.IO;

namespace Element
{
    public interface IHost
    {
        bool ParseFile(in CompilationInput input, in FileInfo file);
        float[] Execute(in CompilationInput input, in string expression, params float[] functionArgs);
    }
}