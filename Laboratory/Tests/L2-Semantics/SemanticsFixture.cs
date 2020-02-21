using System.IO;
using Element;

namespace Laboratory
{
    internal abstract class SemanticsFixture : HostFixture
    {
        protected SemanticsFixture(IHost host, string sourceFileName) : base(host)
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