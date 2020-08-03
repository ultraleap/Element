using System;
using System.Collections.Generic;
using System.IO;

namespace Element
{
    public class CompilerInput
    {
        public CompilerInput(CompilerSource source, CompilerOptions options)
        {
            Source = source;
            Options = options;
        }

        public CompilerSource Source { get; }
        public CompilerOptions Options { get; }
    }
    
    /// <summary>
    /// Input files and packages to be loaded
    /// </summary>
    public class CompilerSource // TODO: Change to record type when available
    {
        public bool ExcludePrelude { get; set; } = false;
        public IReadOnlyList<string> Packages { get; set; } = Array.Empty<string>();
        public IReadOnlyList<FileInfo> ExtraSourceFiles { get; set; } = Array.Empty<FileInfo>();
        public IReadOnlyList<SourceInfo> ExtraElementSource { get; set; } = new List<SourceInfo>();
    }
    
    /// <summary>
    /// Options which change how the compiler operates
    /// </summary>
    public readonly struct CompilerOptions // TODO: Change to record type when available
    {
        public CompilerOptions(bool releaseMode, bool skipValidation, bool noParseTrace, MessageLevel verbosity)
        {
            ReleaseMode = releaseMode;
            SkipValidation = skipValidation;
            NoParseTrace = noParseTrace;
            Verbosity = verbosity;
        }

        public bool ReleaseMode { get; }
        public bool SkipValidation { get; }
        public bool NoParseTrace { get; }
        public MessageLevel Verbosity { get; }
    }
}