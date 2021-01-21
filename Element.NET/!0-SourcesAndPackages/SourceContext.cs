using System.Collections.Generic;
using System.IO;
using System.Linq;
using Element.AST;

namespace Element
{
    /// <summary>
    /// Represents a collection of loaded element packages and sources.
    /// </summary>
    public class SourceContext
    {
        public SourceContext(CompilerOptions? compilerOptions) => CompilerOptions = compilerOptions;

        public GlobalScope GlobalScope { get; } = new GlobalScope();
        public List<StructuralTuple> GeneratedStructuralTuples { get; } = new List<StructuralTuple>();
        public CompilerOptions? CompilerOptions { get; }

        /// <summary>
        /// Create a source context from a compilation input.
        /// </summary>
        public static Result<SourceContext> CreateAndLoad(CompilerInput compilerInput)
        {
            var sourceContext = new SourceContext(compilerInput.Options);
            return sourceContext.LoadCompilerInput(compilerInput).Map(() => sourceContext);
        }

        /// <summary>
        /// Loads an element source intro the source context.
        /// Will fail if the source is a duplicate.
        /// </summary>
        public Result LoadElementSource(SourceInfo info)
        {
            lock (_syncRoot)
            {
                return GlobalScope.AddSource(info, Context.CreateFromSourceContext(this));
            }
        }

        /// <summary>
        /// Loads an element package into the source context.
        /// Will fail if another version of the package is already loaded or any of the packages sources fail to load.
        /// </summary>
        public Result LoadElementPackage(PackageInfo packageInfo)
        {
            lock (_syncRoot)
            {
                var context = Context.CreateFromSourceContext(this);
                if (_loadedPackages.TryGetValue(packageInfo.Name, out var loaded))
                {
                    return context.Trace(EleMessageCode.DuplicateSourceFile, $"Tried to load package {loaded} when {loaded} is already loaded");
                }
                
                var builder = new ResultBuilder(context);
                builder.AppendInfo($"Started loading element package: {packageInfo}");
                foreach (var src in packageInfo.PackageSources)
                {
                    builder.Append(GlobalScope.AddSource(src, context));
                }

                var anyErrors = builder.Messages.Any(msg => msg.MessageLevel >= MessageLevel.Error);
                if (anyErrors)
                {
                    builder.AppendInfo($"Failed to load element package: {packageInfo}");
                }
                else
                {
                    builder.AppendInfo($"Finished loading element package: {packageInfo}");
                    _loadedPackages[packageInfo.Name] = packageInfo;
                }

                return builder.ToResult();
            }
        }
        
        /// <summary>
        /// Loads the prelude version, packages and source files specified in a compiler input into the source context.
        /// Ignores source files from within package directories.
        /// </summary>
        public Result LoadCompilerInput(CompilerInput input)
        {
            lock (_syncRoot)
            {
                var builder = new ResultBuilder(Context.CreateFromSourceContext(this));

                if (input.PreludeVersion != null)
                {
                    builder.Append(input.PackageRegistry.LookupPackage(new PackageSpecifier("Prelude", input.PreludeVersion), builder.Context).Bind(LoadElementPackage));
                }

                foreach (var specifier in input.Packages)
                {
                    builder.Append(input.PackageRegistry.LookupPackage(specifier, builder.Context).Bind(LoadElementPackage));
                }

                bool IsNotWithinPackageDirectory(FileInfo file)
                {
                    var isWithinPackage = false;
                    var directoryInfo = file.Directory;
                    while (directoryInfo.Parent != null && isWithinPackage == false)
                    {
                        isWithinPackage |= directoryInfo.GetFiles($"*{PackageManifest.FileExtension}", SearchOption.TopDirectoryOnly).Any();
                        directoryInfo = directoryInfo.Parent;
                    }

                    return !isWithinPackage;
                }

                foreach (var src in input.SourceFiles.Where(IsNotWithinPackageDirectory))
                {
                    builder.Append(LoadElementSource(SourceInfo.FromFile(src)));
                }

                return builder.ToResult();
            }
        }

        private readonly Dictionary<string, PackageInfo> _loadedPackages = new Dictionary<string, PackageInfo>();
        private readonly object _syncRoot = new object();
    }
}