using System.IO;

namespace Laboratory
{
    internal interface IHost
    {
        bool Parse(HostContext context, FileInfo file);
        float[] Execute(HostContext context, string functionName, params float[] functionArgs);
    }
}