using System;
using System.Collections.Generic;
using System.IO;

namespace Element.CLR
{
    /// <summary>
    /// Contain data for configuring host behaviour
    /// </summary>
    public class HostContext
    {
        public bool IncludePrelude { get; set; }
        public List<DirectoryInfo>? Packages { get; set; }
        public Action<string>? MessageHandler { get; set; }
        public Action<string>? ErrorHandler { get; set; }
    }
}