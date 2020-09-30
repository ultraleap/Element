using Element;
using CommandLine;

namespace Alchemist
{
	[Verb("evaluate", HelpText = "Compile and evaluate an expression.")]
	internal class EvaluateCommand : BaseCommand
	{
		[Option('e', "expression", Required = true, HelpText = "Expression to evaluate.")]
		public string Expression { get; set; }

		protected override bool _skipValidation => false; // Skipping validation during evaluate may cause indirect compiler errors
		protected override bool _noParseTrace => false;

		protected override Result<string> CommandImplementation(CompilerInput input) =>
			new AtomicHost()
				.Evaluate(input, Expression)
				.Map(result => string.Join(" ", result)
				                     .Replace("∞", "Infinity"));
		// Windows terminal replaces ∞ with 8 - see here https://stackoverflow.com/questions/40907417/why-is-infinity-printed-as-8-in-the-windows-10-console
	}
}