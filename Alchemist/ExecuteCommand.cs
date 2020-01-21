using Element;
using Element.CLR;

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

		protected override int CommandImplementation(CompilationInput input)
		{
			var command = new HostCommand(input);
			var result = command.Execute(Function, Arguments.ToArray());

			// TODO: Needs to consume compiler messages somehow

			var asString = string.Join(", ", result).Replace("∞", "Infinity");
			Alchemist.Log(asString);
			return 0;
		}
	}
}