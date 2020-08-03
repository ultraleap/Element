using Element;

namespace Laboratory.Tests.L3.Prelude
{
    internal abstract class PreludeFixture : HostFixture
    {
        protected CompilerInput ValidatedCompilerInput =>
            new CompilerInput(new CompilerSource{ExtraSourceFiles = new[]{GetEleFile("PreludeTestCode")}},
                                 default);
        
        protected CompilerInput NonValidatedCompilerInput =>
            new CompilerInput(new CompilerSource{ExtraSourceFiles = new[]{GetEleFile("PreludeTestCode")}},
                                 new CompilerOptions(default, true, default, default));
    }
}