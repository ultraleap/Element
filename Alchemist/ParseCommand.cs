using System.IO;
using CommandLine;
using Element;

namespace Alchemist
{
    [Verb("parse-file", HelpText = "Parse the given Element file.")]
    internal class ParseFileCommand : BaseCommand
    {
        [Option('f', "filename", Required = true, HelpText = "Element file to parse.")]
        public string FileName { get; set; }

        protected override (int ExitCode, string Result) CommandImplementation(in CompilationInput compilationInput)
            => (0, new AtomicHost().ParseFile(compilationInput, new FileInfo(FileName)).ToString());
    }
}