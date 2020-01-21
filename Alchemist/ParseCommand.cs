using System.Collections.Generic;
using System.IO;
using System.Linq;
using Element.CLR;

namespace Alchemist
{
    using CommandLine;
    using Element;

    [Verb("parse", HelpText = "Parse the given Element file or text.")]
    internal class ParseCommand : BaseCommand
    {
        [Option('f', "files", Required = true, HelpText = "Element files to parse.")]
        public IEnumerable<string> Filenames { get; set; }

        protected override int CommandImplementation(CompilationInput input)
        {
            var command = new HostCommand(input);
            var parsed = command.Parse(Filenames.Select(name => new FileInfo(name)));

            Alchemist.Log(parsed.ToString());

            return 0;
        }
    }
}