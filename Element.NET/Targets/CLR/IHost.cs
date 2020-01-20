using System.IO;
using Ultimately;

namespace Element.CLR
{
    public interface IHost
    {
        Option Parse(HostContext context, FileInfo file);
        Option<float[]> Execute(HostContext context, string functionName, params float[] functionArgs);
    }
}