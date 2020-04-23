using System.IO;
using Element;

namespace Laboratory.Tests.L2.Semantics
{
    internal abstract class SemanticsFixture : HostFixture
    {
        protected SemanticsFixture(string sourceFileName)
        {
            SourceFiles = new[] {GetEleFile(sourceFileName)};
        }
        
        private FileInfo[] SourceFiles { get; }

        protected CompilationInput CompilationInput => new CompilationInput(FailOnError)
        {
            ExcludePrelude = true,
            ExtraSourceFiles = SourceFiles
        };
    }
}