using Element;

namespace Alchemist
{
	using System.Collections.Generic;
	using System.Linq;
	using CommandLine;

	[Verb("evaluate", HelpText = "Compile and evaluate an element expression with arguments, printing the results to standard out.")]
	internal class EvaluateCommand : BaseCommand
	{
		[Option('e', "expression", Required = true, HelpText = "Expression to evaluate.")]
		public string Function { get; set; }

		[Option('a', "arguments", Required = false, HelpText = "Arguments to evaluate the expression with as a flattened array of floats. Must be same length as expression expects.")]
		public IEnumerable<float> Arguments { get; set; }

		protected override (int ExitCode, string Result) CommandImplementation(in CompilationInput input) =>
			(0, string.Join(", ", new AtomicHost().Evaluate(input, Function, Arguments.ToArray()))
				.Replace("∞", "Infinity"));
		// Windows terminal replaces ∞ with 8 - see here https://stackoverflow.com/questions/40907417/why-is-infinity-printed-as-8-in-the-windows-10-console
	}
}