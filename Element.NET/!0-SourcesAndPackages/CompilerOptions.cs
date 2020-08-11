namespace Element
{
    /// <summary>
    /// Options which change how the compiler operates.
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