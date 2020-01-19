using System.IO;

namespace Alchemist
{
    using CommandLine;
    using Element;

    [Verb("parse", HelpText = "Parse the given Element file or text.")]
    internal class ParseCommand : BaseCommand
    {
        [Option('f', "filename", Required = true, HelpText = "Element file to parse.")]
        public string Filename { get; set; }

        protected override CompilationContext _compilationContext { get; } = new CompilationContext();

        protected override int CommandImplementation()
        {
            _sourceContext.AddSourceFiles(new []{new FileInfo(Filename)});
            _sourceContext.Recompile(_compilationContext);
            
            var asString = string.Join(", ", output).Replace("∞", "Infinity");
            Alchemist.Log(asString);
            return 0;
        }
    }
}