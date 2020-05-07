using Element;

namespace Laboratory.Tests.L3.Prelude
{
    internal abstract class PreludeFixture : HostFixture
    {
        protected PreludeFixture(string file = default) => SourceFiles = string.IsNullOrEmpty(file) ? Array.Empty<FileInfo>() : new[]{GetEleFile(file)};
        private FileInfo[] SourceFiles { get; }
        protected CompilationInput ValidatedCompilationInput => new CompilationInput(LogMessage) {ExtraSourceFiles = SourceFiles};
        protected CompilationInput NonValidatedCompilationInput => new CompilationInput(LogMessage) {SkipValidation = true, ExtraSourceFiles = SourceFiles};
        protected static CompilationInput CompilationInput => new CompilationInput(FailOnError)
        {
            ExtraSourceFiles = new[]{GetEleFile("PreludeTestCode")}
        };
    }
}