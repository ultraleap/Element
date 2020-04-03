using System.IO;
using Element;

namespace Laboratory.Tests
{
    internal abstract class StandardLibraryFixture : HostFixture
    {
        protected CompilationInput ValidatedCompilationInput { get; } = new CompilationInput(FailOnError)
        {
            Packages = new[] { new DirectoryInfo("StandardLibrary") },
        };
        
        protected CompilationInput NonValidatedCompilationInput { get; } = new CompilationInput(FailOnError)
        {
            Packages = new[] { new DirectoryInfo("StandardLibrary") },
            SkipValidation = true
        };
    }
}