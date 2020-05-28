using System;
using System.Collections.Generic;
using System.IO;

namespace Element
{
    /// <summary>
    /// Contain input data for compiler including files to compile and compiler flags
    /// </summary>
    public class CompilationInput // TODO: Change to record type when available
    {
        public CompilationInput(Action<CompilerMessage>? logCallback) { LogCallback = logCallback; }
        public Action<CompilerMessage>? LogCallback { get; set; }
        public bool ExcludePrelude { get; set; } = false;
        public IReadOnlyList<DirectoryInfo> Packages { get; set; } = Array.Empty<DirectoryInfo>();
        public IReadOnlyList<FileInfo> ExtraSourceFiles { get; set; } = Array.Empty<FileInfo>();
        public bool Debug { get; set; } = true;
        public bool SkipValidation { get; set; } = false;
        public bool NoParseTrace { get; set; } = false;
        public MessageLevel Verbosity { get; set; } = MessageLevel.Information;
    }
}