using CommandLine;
using Element;

namespace Alchemist
{
    [Verb("typeof", HelpText = "Determine the type of value returned by a given expression.")]
    internal class TypeofCommand : BaseCommand
    {
        [Option('e', "expression", Required = true, HelpText = "Expression to evaluate.")]
        public string Expression { get; set; }

        protected override bool _skipValidation => false;
        protected override bool _noParseTrace => false;

        protected override Result<string> CommandImplementation(CompilerInput input) => new AtomicHost().Typeof(input, Expression);
    }
}