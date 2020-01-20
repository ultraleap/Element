using System.IO;
using Element.CLR;

namespace Alchemist
{
    using CommandLine;
    using Element;

    [Verb("parse", HelpText = "Parse the given Element file or text.")]
    internal class ParseCommand : BaseCommand
    {
        [Option('f', "filename", Required = true, HelpText = "Element file to parse.")]
        public string Filename { get; set; }

        protected override CompilationContext CompilationContext { get; } = new CompilationContext();

        protected override  CommandImplementation()
        {
            Alchemist.Log(asString);
            return 0;
        }
    }
}