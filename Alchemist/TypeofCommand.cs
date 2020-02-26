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

        protected override (int ExitCode, string Result) CommandImplementation(in CompilationInput input) =>
            (0, new AtomicHost().Typeof(input, Expression) switch
            {
                (true, {} result) => result,
                _ => "<error>"
            });
    }
}