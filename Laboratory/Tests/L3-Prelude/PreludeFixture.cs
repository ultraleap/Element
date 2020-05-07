using Element;

namespace Laboratory.Tests.L3.Prelude
{
    internal abstract class PreludeFixture : HostFixture
    {
        protected CompilationInput ValidatedCompilationInput => new CompilationInput(LogMessage) {ExtraSourceFiles = new[]{GetEleFile("PreludeTestCode")}};
        protected CompilationInput NonValidatedCompilationInput => new CompilationInput(LogMessage) {SkipValidation = true};
    }
}