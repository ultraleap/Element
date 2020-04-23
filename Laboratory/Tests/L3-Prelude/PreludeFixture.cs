using System;
using System.IO;
using Element;

namespace Laboratory.Tests.L3.Prelude
{
    internal abstract class PreludeFixture : HostFixture
    {
        protected PreludeFixture(string file = default) => SourceFiles = string.IsNullOrEmpty(file) ? Array.Empty<FileInfo>() : new[]{GetEleFile(file)};
        private FileInfo[] SourceFiles { get; }
        protected CompilationInput ValidatedCompilationInput => new CompilationInput(FailOnError) {ExtraSourceFiles = SourceFiles};
        protected CompilationInput NonValidatedCompilationInput => new CompilationInput(FailOnError) {SkipValidation = true, ExtraSourceFiles = SourceFiles};
    }
}