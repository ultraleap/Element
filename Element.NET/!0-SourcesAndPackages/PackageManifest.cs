using System;
using Newtonsoft.Json;
using ResultNET;

namespace Element
{
    /// <summary>
    /// The json manifest file of an element package (.bond).
    /// </summary>
    public readonly struct PackageManifest
    {
        public static readonly string FileExtension = ".bond";
        
        public PackageManifest(string version)
        {
            Version = version;
        }
            
        public static Result<PackageManifest> ParseFromJsonString(string json, ITraceContext context)
        {
            try
            {
                return JsonConvert.DeserializeObject<PackageManifest>(json);
            }
            catch (Exception e)
            {
                return context.Trace(ElementMessage.ParseError, e.ToString());
            }
        }

        public string Version { get; }
    }
}