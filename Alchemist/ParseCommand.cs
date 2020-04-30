using CommandLine;
using Element;

namespace Alchemist
{
    [Verb("parse", HelpText = "Parse the given Element file and return success or failure.")]
    internal class ParseCommand : BaseCommand
    {
        [Option("no-validation", Required = false, Default = false, HelpText = "Skip validating files after parsing. Only syntax correctness will be checked. Issues such as invalid/duplicate identifiers will not be caught.")]
        public bool SkipValidation { get; set; }

        protected override bool _skipValidation => SkipValidation;

        protected override (int ExitCode, string Result) CommandImplementation(CompilationInput compilationInput)
            => (0, new AtomicHost().Parse(compilationInput).ToString());
    }
}