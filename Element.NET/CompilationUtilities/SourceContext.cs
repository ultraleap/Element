using System;
using System.IO;
using System.Linq;
using Element.AST;

namespace Element
{
    public class SourceContext : Context
    {
        private SourceContext(CompilationInput compilationInput) : base(new GlobalScope(), compilationInput) { }

        public static bool TryCreate(CompilationInput compilationInput, out SourceContext sourceContext)
        {
            sourceContext = new SourceContext(compilationInput);
            return sourceContext.LoadPackagesAndExtraSourceFiles();
        }

        protected override CompilerMessage MakeMessage(int? messageCode, string context = default)=> !messageCode.HasValue
            ? new CompilerMessage(null, null, context, null)
            : new CompilerMessage(messageCode.Value, CompilerMessage.GetMessageLevel(messageCode.Value), context, null);

        private static readonly LambdaEqualityComparer<DirectoryInfo> _directoryComparer = new LambdaEqualityComparer<DirectoryInfo>((a, b) => a.FullName == b.FullName, info => info.GetHashCode());
        private static readonly LambdaEqualityComparer<FileInfo> _fileComparer = new LambdaEqualityComparer<FileInfo>((a, b) => a.FullName == b.FullName, info => info.GetHashCode());

        private static readonly object _syncRoot = new object();

        public CompilationContext MakeCompilationContext() => new CompilationContext(GlobalScope, CompilationInput);

        public bool ApplyExtraInput(CompilationInput input)
        {
            if (CompilationInput == input) return true;
            lock (_syncRoot)
            {
                CompilationInput.Packages = CompilationInput.Packages.Union(input.Packages, _directoryComparer).ToArray();
                CompilationInput.ExtraSourceFiles = CompilationInput.ExtraSourceFiles.Union(input.ExtraSourceFiles, _fileComparer).ToArray();

                return LoadPackagesAndExtraSourceFiles();
            }
        }

        private bool LoadPackagesAndExtraSourceFiles()
        {
            lock (_syncRoot)
            {
                //SearchOption.AllDirectories temporarily switched to SearchOption.TopDirectoryOnly,
                //so that I can move files still to be updated to a subdirectory
                return this.ParseFiles(CompilationInput.Packages
                        .Prepend(CompilationInput.ExcludePrelude ? null : new DirectoryInfo("Prelude"))
                        .SelectMany(directory =>
                            directory?.GetFiles("*.ele", SearchOption.TopDirectoryOnly) ?? Array.Empty<FileInfo>())
                        .Concat(CompilationInput.ExtraSourceFiles)
                        .ToArray())
                    .OverallSuccess;
            }
        }

        /*/// <summary>
        /// Gets all functions in global scope and any namespaces which match the given filter.
        /// </summary>
        public (string Path, IFunction Function)[] GetAllFunctions(Predicate<IFunction> filter, CompilationContext context)
        {
	        IEnumerable<(string, IFunction)> Recurse(string path, IFunction func)
	        {
		        if (func.IsNamespace())
		        {
			        return func.Outputs.SelectMany(o => Recurse($"{path}.{o.Name}", func.Call(o.Name, context)));
		        }

		        return filter?.Invoke(func) == false ? Array.Empty<(string, IFunction)>() : new[] {(path, func)};
	        }

	        return _functions.ToArray().SelectMany(f => Recurse(f.Name, f)).ToArray();
        }*/
    }
}