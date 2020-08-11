using System;
using System.IO;
using System.Linq;
using Element;

namespace Laboratory.Tests.L2.Semantics
{
    internal abstract class SemanticsFixture : HostFixture
    {
        protected SemanticsFixture(params string[] sourceFileName) => SourceFiles = sourceFileName.Select(GetEleFile).ToArray();

        private FileInfo[] SourceFiles { get; }

        protected CompilerInput CompilerInput => new CompilerInput(TestPackageRegistry, null,  Array.Empty<PackageSpecifier>(), SourceFiles, default);
    }
}