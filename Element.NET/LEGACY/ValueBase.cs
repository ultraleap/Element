using System.IO;

namespace Element
{
	using System;
	using System.Linq;
	using Eto.Parse;

	/// <summary>
	/// Base class for Element values (i.e. functions and types)
	/// </summary>
	internal abstract class ValueBase : IFunction, INamedItem
	{
		protected ValueBase(ICompilationScope parent, Match ast, CompilationContext context, FileInfo source)
		{
			Source = source;
			Parent = parent;
			Ast = ast ?? throw new ArgumentNullException(nameof(ast));
			Name = ast[ElementAST.FunctionName].Text;
			_context = context;
		}

		private readonly CompilationContext _context;
		protected readonly ICompilationScope Parent;
		protected readonly Match Ast;
		protected readonly FileInfo Source;

		public string Name { get; }
		public override string ToString() => Name;

		private PortInfo ParsePort(Match m, bool isReturn, CompilationContext info)
		{
			var ret = new PortInfo
			{
				Name = isReturn ? "return" : m[ElementAST.PortName].Text,
				Type = m[ElementAST.PortType] ? Parent.FindType(m[ElementAST.PortType].Text, info) : AnyType.Instance
			};

			if (ret.Type == null)
			{
				info.LogError(7, $"{this}: Type {m[ElementAST.PortType]} not found");
				ret.Type = AnyType.Instance;
			}

			return ret;
		}

		private PortInfo[] ParsePorts(Match portsList, CompilationContext info) =>
			portsList.Matches.Select(m => ParsePort(m, false, info)).ToArray();

		public abstract IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context);

		// TODO: Think about how to pass CompilationContext into here
		// Performance is vital here, hence the caching.
		private PortInfo[] _inputs, _outputs;

		public PortInfo[] Inputs => _inputs ??= ParsePorts(Ast[ElementAST.FunctionInputs], _context);

		public PortInfo[] Outputs => _outputs ??= ParseOutputPorts();

		private PortInfo[] ParseOutputPorts() =>
			Ast[ElementAST.FunctionOutputs]
				? ParsePorts(Ast[ElementAST.FunctionOutputs], _context)
				: new[] {ParsePort(Ast, true, _context)};
	}
}