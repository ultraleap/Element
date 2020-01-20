using System.IO;

namespace Element.CLR
{
    public interface IHost
    {
        CompilationResult<bool> Parse(CompilationInput input, FileInfo file);
        CompilationResult<float[]> Execute(CompilationInput input, string functionName, params float[] functionArgs);
    }
}