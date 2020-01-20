using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Element.CLR
{
    public class HostCommand
    {
        public HostCommand(CompilationInput compilationInput)
        {
            CompilationContext =new CompilationContext(compilationInput);
            static FileInfo[] OpenPackage(DirectoryInfo package) => package.GetFiles("*.ele", SearchOption.AllDirectories);

            if (compilationInput.ExcludePrelude) SourceFiles.AddRange(OpenPackage(new DirectoryInfo("Prelude")));
            SourceFiles.AddRange(compilationInput.Packages.SelectMany(OpenPackage));
        }

        private List<FileInfo> SourceFiles { get; } = new List<FileInfo>();
        private CompilationContext CompilationContext { get; }

        public CompilationResult<bool> Parse(in FileInfo file)
        {
            new GlobalScope().ParseFile(file, CompilationContext);
            return CompilationContext.ToResult(CompilationContext.Messages.Any(m => m.Level >= MessageLevel.Error));
        }

        public CompilationResult<float[]> Execute(in string functionName, params float[] functionArgs) =>
            CompilationContext.ToResult(
                new GlobalScope().ParseFiles(SourceFiles, CompilationContext)
                                 .GetFunction(functionName, CompilationContext)
                                 .EvaluateAndSerialize(functionArgs, CompilationContext));
    }
}