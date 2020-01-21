using System.Collections.Generic;
using System.IO;

namespace Element.CLR
{
    public interface IHost
    {
        bool ParseFiles(in CompilationInput input, in IEnumerable<FileInfo> file);
        float[] Execute(in CompilationInput input, in string functionName, params float[] functionArgs);
    }
}