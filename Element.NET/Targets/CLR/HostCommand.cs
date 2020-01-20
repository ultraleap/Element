using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Ultimately;
using Ultimately.Reasons;

namespace Element.CLR
{
    public class HostCommand
    {
        public HostCommand(HostContext hostContext, CompilationContext compilationContext = default)
        {
            CompilationContext = compilationContext ??= new CompilationContext{LogToConsole = false};

            static FileInfo[] OpenPackage(DirectoryInfo package) => package.GetFiles("*.ele", SearchOption.AllDirectories);

            if (hostContext.IncludePrelude) SourceFiles.AddRange(OpenPackage(new DirectoryInfo("Prelude")));
            SourceFiles.AddRange(hostContext.Packages.SelectMany(OpenPackage));

            compilationContext.OnLog += hostContext.MessageHandler.Invoke;
            compilationContext.OnError += hostContext.ErrorHandler.Invoke;

            sourceContext.ParseAndAddTo(compilationContext);
        }

        private List<FileInfo> SourceFiles { get; }
        private CompilationContext CompilationContext { get; }

        public Option Parse(in FileInfo file)
        {
            new GlobalScope().ParseFile(CompilationContext, file);
            return Optional.SomeWhen(CompilationContext.Messages.Any(m => m.Level >= MessageLevel.Error),
                Error.Create(CompilationContext.Messages.Aggregate(new StringBuilder(), (builder, message) => builder.AppendLine(message.ToString())).ToString()));
        }

        public Option<float[]> Execute(in string functionName, params float[] functionArgs)
        {

            var result = new GlobalScope().GetFunction(functionName, CompilationContext).EvaluateAndSerialize(functionArgs, CompilationContext);
            return Optional.Some(result);
        }
    }
}