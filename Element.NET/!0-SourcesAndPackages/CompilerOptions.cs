using ResultNET;

namespace Element
{
    /// <summary>
    /// Flags which change how the compiler operates.
    /// </summary>
    public class CompilerOptions // TODO: Change to record type when available
    {
        public CompilerOptions(MessageLevel verbosity,
            bool releaseMode = false,
            bool skipValidation = false,
            bool noParseTrace = false,
            int callStackLimit = 1000)
        {
            ReleaseMode = releaseMode;
            SkipValidation = skipValidation;
            NoParseTrace = noParseTrace;
            Verbosity = verbosity;
            CallStackLimit = callStackLimit;
        }

        public bool ReleaseMode { get; }
        public bool SkipValidation { get; }
        public bool NoParseTrace { get; }
        public MessageLevel Verbosity { get; }
        public int CallStackLimit { get; }
    }
}