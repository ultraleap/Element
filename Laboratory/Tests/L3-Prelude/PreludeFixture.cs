using Element;

namespace Laboratory.Tests.L3.Prelude
{
    internal abstract class PreludeFixture : HostFixture
    {
        protected CompilationInput ValidatedCompilationInput => new CompilationInput {ExtraSourceFiles = new[]{GetEleFile("PreludeTestCode")}};
        protected CompilationInput NonValidatedCompilationInput => new CompilationInput {SkipValidation = true};
    }
}