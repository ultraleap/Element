using CommandLine;
using Element;
using ResultNET;

namespace Alchemist
{
    [Verb("parse", HelpText = "Parse the given Element file and return success or failure.")]
    internal class ParseCommand : BaseCommand
    {
        [Option("no-validation", Required = false, Default = false, HelpText = "Skip validating files after parsing. Only syntax correctness will be checked. Issues such as invalid/duplicate identifiers will not be caught.")]
        public bool SkipValidation { get; set; }
        
        [Option("no-parse-trace", Required = false, Default = false, HelpText = "Disable parse trace. Can dramatically improve performance in situations such as tests that are expected to fail.")]
        public bool NoParseTrace { get; set; }

        protected override bool _skipValidation => SkipValidation;
        protected override bool _noParseTrace => NoParseTrace;

        protected override Result<string> CommandImplementation(CompilerInput compilerInput) =>
            new AtomicHost().Parse(compilerInput).Map(() => "Successfully parsed");
    }
}