using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Element.AST;
using Newtonsoft.Json;

namespace Element
{
    /// <summary>
    /// Represents a bundle of loaded source files.
    /// </summary>
    public class SourceContext : TraceBase
    {
        private SourceContext(CompilationInput compilationInput)
        {
            CompilationInput = compilationInput;
        }
        
        public GlobalScope GlobalScope { get; } = new GlobalScope();
        public CompilationInput CompilationInput { get; }
        

        public static Result<SourceContext> Create(CompilationInput compilationInput)
        {
            var context = new SourceContext(compilationInput);
            return context.LoadPackagesAndExtraSourceFiles().Map(() => context);
        }

        public Result<IValue> EvaluateExpression(string expression, IScope? scopeToEvaluateIn = null) =>
            Parser.Parse<AST.Expression>(new SourceInfo("<evaluated expression>", expression), this, CompilationInput.NoParseTrace)
                  .Check(expressionObject => expressionObject.Validate(new CompilationContext(this)))
                  .Bind(expressionObject => expressionObject.ResolveExpression(scopeToEvaluateIn ?? GlobalScope, new CompilationContext(this)));

        public Result<SourceContext> LoadElementSourceFile(FileInfo file) => LoadElementSourceString(new SourceInfo(file.FullName, File.ReadAllText(file.FullName)));
        public Result<SourceContext> LoadElementSourceString(SourceInfo info) => GlobalScope.AddSource(info, this).Map(() => this);

        /// <summary>
        /// Parses all the given files as Element source files into the source context
        /// </summary>
        public Result<SourceContext> LoadElementSourceFiles(IEnumerable<FileInfo> files) => files.Select(LoadElementSourceFile).Fold().Map(() => this);
        
        public Result ApplyExtraInput(CompilationInput input)
        {
            if (CompilationInput == input) return Result.Success; // CompilationInput is immutable so this is a no-op
            lock (_syncRoot)
            {
                CompilationInput.Packages = CompilationInput.Packages.Union(input.Packages).ToArray();
                CompilationInput.ExtraSourceFiles = CompilationInput.ExtraSourceFiles.Union(input.ExtraSourceFiles, _fileComparer).ToArray();

                return LoadPackagesAndExtraSourceFiles();
            }
        }

        private static readonly LambdaEqualityComparer<FileInfo> _fileComparer = new LambdaEqualityComparer<FileInfo>((a, b) => a.FullName == b.FullName, info => info.GetHashCode());
        private static readonly object _syncRoot = new object();

        private struct PackageManifest
        {
            public PackageManifest(string name)
            {
                Name = name;
            }

            public string Name { get; }
        }
        
        private Result LoadPackagesAndExtraSourceFiles()
        {
            lock (_syncRoot)
            {
                var builder = new ResultBuilder(this);
                
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
                var packageManifests = currentDir.GetFiles("*.elepkg", SearchOption.AllDirectories)
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

                var packagesNamesToLoad = CompilationInput.ExcludePrelude
                                              ? CompilationInput.Packages
                                              : CompilationInput.Packages.Prepend("Prelude");

                var packagesToLoad = packagesNamesToLoad.Select(pkgName =>
                                                        {
                                                            if (!packageToDirectoryMap.TryGetValue(pkgName, out var pkgDir))
                                                            {
                                                                builder.Append(MessageCode.FileAccessError, $"No package '{pkgName}' found");
                                                            }

                                                            return pkgDir;
                                                        });

                builder.Append(LoadElementSourceFiles(packagesToLoad
                                                      .SelectMany(pkg => pkg.GetFiles("*.ele", SearchOption.AllDirectories))
                                                      .Concat(CompilationInput.ExtraSourceFiles)
                                                      .Distinct(_fileComparer)));
                return builder.ToResult();
            }
        }

        public override MessageLevel Verbosity => CompilationInput.Verbosity;
        public override IReadOnlyCollection<TraceSite>? TraceStack => null;
    }
}