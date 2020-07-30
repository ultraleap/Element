using System.IO;
using System.Linq;
using Element;

namespace Laboratory.Tests.L2.Semantics
{
    internal abstract class SemanticsFixture : HostFixture
    {
        protected SemanticsFixture(params string[] sourceFileName) => SourceFiles = sourceFileName.Select(GetEleFile).ToArray();

        private FileInfo[] SourceFiles { get; }

        protected CompilationInput CompilationInput => new CompilationInput
        {
            ExcludePrelude = true,
            ExtraSourceFiles = SourceFiles
        };
    }
}