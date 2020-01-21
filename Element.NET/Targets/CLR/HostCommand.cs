using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Element.CLR
{
    public class HostCommand
    {
        public HostCommand(CompilationInput compilationInput)
        {
            CompilationContext = new CompilationContext(compilationInput);
            static FileInfo[] OpenPackage(DirectoryInfo package) => package.GetFiles("*.ele", SearchOption.AllDirectories);

            if (compilationInput.ExcludePrelude) SourceFiles.AddRange(OpenPackage(new DirectoryInfo("Prelude")));
            SourceFiles.AddRange(compilationInput.Packages.SelectMany(OpenPackage));
        }

        private List<FileInfo> SourceFiles { get; } = new List<FileInfo>();
        public CompilationContext CompilationContext { get; }

        public bool Parse(in IEnumerable<FileInfo> files) => CompilationContext.ParseFiles(files).Messages.Any(m => m.Level >= MessageLevel.Error);

        public float[] Execute(in string functionName, params float[] functionArgs) =>
            CompilationContext.ParseFiles(SourceFiles).GlobalScope
                .GetFunction(functionName, CompilationContext)
                .EvaluateAndSerialize(functionArgs, CompilationContext);
    }
}