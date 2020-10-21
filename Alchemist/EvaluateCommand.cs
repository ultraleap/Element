using System.Collections.Generic;
using Element;
using CommandLine;
using CommandLine.Text;

namespace Alchemist
{
	[Verb("evaluate", HelpText = "Evaluate an Element expression.")]
	internal class EvaluateCommand : BaseCommand
	{
		[Option('e', "expression", Required = true, HelpText = "Expression to evaluate.")]
		public string Expression { get; set; }
		
		[Option('a', "arguments", Required = false, HelpText = "Arguments for when expression resolves to a function, in the format of an element call expression.")]
		public string Arguments { get; set; }
		
		[Option("interpreted", Required = false, HelpText = "Interpret the expression/function (with arguments), returning the result serialized. Default behaviour will compile the expression dynamically - if arguments are supplied they are compiled separately and expression called dynamically.")]
		public bool Interpreted { get; set; }

		protected override bool _skipValidation => false; // Skipping validation during evaluate may cause indirect compiler errors
		protected override bool _noParseTrace => false;
		
		[Usage(ApplicationAlias = "Alchemist")]
		public static IEnumerable<Example> Examples =>
			new List<Example>
			{
				new Example("Evaluate adding expression inline", new EvaluateCommand {Expression = "5.add(10)"}),
				new Example("Compiled add and call with arguments", new EvaluateCommand {Expression = "Num.add", Arguments = "(5, 10)"}),
				new Example("Evaluate add as an interpreted call", new EvaluateCommand {Expression = "Num.add", Arguments = "(5, 10)", Interpreted = true}),
			};

		protected override Result<string> CommandImplementation(CompilerInput input) =>
			(string.IsNullOrEmpty(Arguments)
				 ? new AtomicHost().EvaluateExpression(input, Expression, Interpreted)
				 : new AtomicHost().EvaluateFunction(input, Expression, Arguments, Interpreted))
			.Map(result => string.Join(" ", result)
			                     .Replace("∞", "Infinity"));

		// Windows terminal replaces ∞ with 8 - see here https://stackoverflow.com/questions/40907417/why-is-infinity-printed-as-8-in-the-windows-10-console
	}
}