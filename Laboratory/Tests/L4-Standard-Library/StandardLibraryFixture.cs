using System;
using System.IO;
using Element;
using Element.NET.TestHelpers;
using Range = SemVer.Range;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal abstract class StandardLibraryFixture : HostFixture
    {
        protected CompilerInput ValidatedCompilerInput { get; } = new CompilerInput(
            TestPackageRegistry,
            new[] {new PackageSpecifier("StandardLibrary", new Range("*"))},
            Array.Empty<FileInfo>(),
            new CompilerOptions(default));

        protected CompilerInput NonValidatedCompilerInput { get; } = new CompilerInput(
            TestPackageRegistry,
            new[] {new PackageSpecifier("StandardLibrary", new Range("*"))},
            Array.Empty<FileInfo>(),
            new CompilerOptions(default, default, true));
    }
}