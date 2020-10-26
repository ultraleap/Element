using System.Collections.Generic;
using System.IO;
using SemVer;

namespace Element
{
    /// <summary>
    /// Aggregate of all the input to be given into the compiler to perform compilation.
    /// Typically used to initialize a <see cref="SourceContext"/>.
    /// </summary>
    public class CompilerInput
    {
        public CompilerInput(IPackageRegistry packageRegistry,
                             IReadOnlyList<PackageSpecifier> packages,
                             IReadOnlyList<FileInfo> sourceFiles,
                             CompilerOptions options)
            : this(packageRegistry,
                   new Range(string.Empty), // Match any version, will pick highest
                packages, sourceFiles, options) {}
        
        public CompilerInput(IPackageRegistry packageRegistry,
                             Range? preludeVersion,
                             IReadOnlyList<PackageSpecifier> packages,
                             IReadOnlyList<FileInfo> sourceFiles,
                             CompilerOptions options)
        {
            PackageRegistry = packageRegistry;
            PreludeVersion = preludeVersion;
            Options = options;
            Packages = packages;
            SourceFiles = sourceFiles;
        }

        public IPackageRegistry PackageRegistry { get; }
        public Range? PreludeVersion { get; }
        public IReadOnlyList<PackageSpecifier> Packages { get; }
        public IReadOnlyList<FileInfo> SourceFiles { get; }
        public CompilerOptions Options { get; }
    }
}