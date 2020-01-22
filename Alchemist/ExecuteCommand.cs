using Element;

namespace Alchemist
{
	using System.Collections.Generic;
	using System.Linq;
	using CommandLine;

	[Verb("execute", HelpText = "Compile and execute an element function with arguments, printing the results to standard out.")]
	internal class ExecuteCommand : BaseCommand
	{
		[Option('f', "function", Required = true, HelpText = "Function to execute.")]
		public string Function { get; set; }

		[Option('a', "arguments", Required = false, HelpText = "Arguments to execute the function with as a flattened array of floats. Must be same length as function expects.")]
		public IEnumerable<float> Arguments { get; set; }

		protected override (int ExitCode, string Result) CommandImplementation(in CompilationInput input) =>
			(0, string.Join(", ", new AtomicHost().Execute(input, Function, Arguments.ToArray()))
				.Replace("∞", "Infinity"));
		// Windows terminal replaces ∞ with 8 - see here https://stackoverflow.com/questions/40907417/why-is-infinity-printed-as-8-in-the-windows-10-console
	}
}