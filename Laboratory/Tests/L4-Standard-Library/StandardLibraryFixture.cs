using Element;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal abstract class StandardLibraryFixture : HostFixture
    {
        protected CompilerInput ValidatedCompilerInput { get; } = new CompilerInput(
            new CompilerSource
            {
                Packages = new[] {"StandardLibrary"},
            }, default);

        protected CompilerInput NonValidatedCompilerInput { get; } = new CompilerInput(
            new CompilerSource
        {
            Packages = new[] { "StandardLibrary" }
        }, new CompilerOptions(default, true, default, default));
    }
}