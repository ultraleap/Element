using System;
using System.IO;
using Element;
using Range = SemVer.Range;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal abstract class StandardLibraryFixture : HostFixture
    {
        protected CompilerInput ValidatedCompilerInput { get; } = new CompilerInput(
            TestPackageRegistry,
            new[] {new PackageSpecifier("StandardLibrary", new Range("*"))},
            Array.Empty<FileInfo>(),
            default);

        protected CompilerInput NonValidatedCompilerInput { get; } = new CompilerInput(
            TestPackageRegistry,
            new[] {new PackageSpecifier("StandardLibrary", new Range("*"))},
            Array.Empty<FileInfo>(),
            new CompilerOptions(default, true, default, default));
    }
}