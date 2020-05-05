using Element;

namespace Laboratory.Tests.L3.Prelude
{
    internal abstract class PreludeFixture : HostFixture
    {
        protected static CompilationInput CompilationInput => new CompilationInput(FailOnError)
        {
            ExtraSourceFiles = new[]{GetEleFile("PreludeTestCode")}
        };
    }
}