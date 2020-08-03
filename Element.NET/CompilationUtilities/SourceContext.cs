using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Element.AST;
using Newtonsoft.Json;

namespace Element
{
    /// <summary>
    /// Represents a bundle of loaded element source
    /// </summary>
    public class SourceContext
    {
        private SourceContext(CompilerOptions compilerOptions)
        {
            CompilerOptions = compilerOptions;
        }
        
        public GlobalScope GlobalScope { get; } = new GlobalScope();
        public CompilerOptions CompilerOptions { get; }

        public static Result<SourceContext> CreateAndLoad(CompilerInput compilerInput)
        {
            var context = new SourceContext(compilerInput.Options);
            return context.LoadInputFiles(compilerInput.Source).Map(() => context);
        }

        public static SourceContext Create(CompilerOptions compilerOptions) => new SourceContext(compilerOptions);

        public Result<SourceContext> LoadElementSourceFile(FileInfo file) => LoadElementSourceString(new SourceInfo(file.FullName, File.ReadAllText(file.FullName)));
        public Result<SourceContext> LoadElementSourceString(SourceInfo info) => GlobalScope.AddSource(info, new Context(this)).Map(() => this);

        /// <summary>
        /// Parses all the given files as Element source files into the source context
        /// </summary>
        public Result<SourceContext> LoadElementSourceFiles(IEnumerable<FileInfo> files)
        {
            files = files as FileInfo[] ?? files.ToArray(); // Ensure multiple enumeration isn't an issue
            var alreadyLoadedFiles = files.Where(f => GlobalScope.ContainsSource(f.FullName)).ToArray();
            var context = new Context(this);
            var alreadyLoadedMsgs = alreadyLoadedFiles.Select(f => context.Trace(MessageLevel.Information, $"Skipping loading '{f.FullName}' as a source with this name is already loaded")).ToArray();
            return files.Except(alreadyLoadedFiles).Select(LoadElementSourceFile).Fold().Bind(() => new Result<SourceContext>(this, alreadyLoadedMsgs));
        }

        private static readonly LambdaEqualityComparer<FileInfo> _fileComparer = new LambdaEqualityComparer<FileInfo>((a, b) => a.FullName == b.FullName, info => info.GetHashCode());
        private static readonly object _syncRoot = new object();

        private readonly struct PackageManifest
        {
            public PackageManifest(string name)
            {
                Name = name;
            }

            public string Name { get; }
        }
        
        public Result LoadInputFiles(CompilerSource source)
        {
            lock (_syncRoot)
            {
                var builder = new ResultBuilder(new Context(this));
                
                PackageManifest? ParseFromJsonString(string json)
                {
                    try
                    {
                        return JsonConvert.DeserializeObject<PackageManifest>(json);
                    }
                    catch (Exception e)
                    {
                        builder.Append(MessageCode.ParseError, e.ToString());
                        return null;
                    }
                }
                
                var currentDir = new DirectoryInfo(Directory.GetCurrentDirectory());
                var packageManifests = currentDir.GetFiles("*.bond", SearchOption.AllDirectories)
                                                 .ToDictionary(f => f, f => ParseFromJsonString(File.ReadAllText(f.FullName)));
                var packageToDirectoryMap = new Dictionary<string, DirectoryInfo>();
                foreach (var package in packageManifests)
                {
                    if (!package.Value.HasValue) continue;
                    if (packageToDirectoryMap.ContainsKey(package.Value.Value.Name))
                    {
                        builder.Append(MessageCode.DuplicateSourceFile, $"Multiple definitions for package '{package.Value.Value.Name}'");
                    }
                    else
                    {
                        packageToDirectoryMap.Add(package.Value.Value.Name, package.Key.Directory);
                    }
                }

                var packagesNamesToLoad = source.ExcludePrelude
                                              ? source.Packages
                                              : source.Packages.Prepend("Prelude");

                var packagesToLoad = packagesNamesToLoad.Select(pkgName =>
                                                        {
                                                            if (!packageToDirectoryMap.TryGetValue(pkgName, out var pkgDir))
                                                            {
                                                                builder.Append(MessageCode.FileAccessError, $"No package '{pkgName}' found");
                                                            }

                                                            return pkgDir;
                                                        });

                // Load package source files and extra source files
                builder.Append(LoadElementSourceFiles(packagesToLoad
                                                      .SelectMany(pkg => pkg.GetFiles("*.ele", SearchOption.AllDirectories))
                                                      .Concat(source.ExtraSourceFiles)
                                                      .Distinct(_fileComparer)));
                
                // Load extra element source blobs
                foreach (var src in source.ExtraElementSource)
                {
                    builder.Append( LoadElementSourceString(src));
                }
                
                return builder.ToResult();
            }
        }
    }
}