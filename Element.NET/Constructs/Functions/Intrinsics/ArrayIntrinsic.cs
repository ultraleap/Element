namespace Element
{
	using System.Linq;

	internal class ArrayIntrinsic : INamedFunction
	{
		public string Name => "array";
		public PortInfo[] Inputs => null;

		public PortInfo[] Outputs { get; } = {
			new PortInfo{Name = "count", Type = NumberType.Instance},
			new PortInfo{Name = "index", Type = AnyType.Instance}
		};

		public IFunction CallInternal(IFunction[] arguments, string name, CompilationContext context)
		{
			if (arguments.Length == 0)
			{
				return context.LogError("ELE2000");
			}

			switch (name)
			{
				case "count": return new Constant(arguments.Length);
				case "index": return new IndexFunction(arguments);
				default:
					return context.LogError($"No output named {name}");
			}
		}

		public class IndexFunction : IFunction
		{
			public IndexFunction(IFunction[] elements)
			{
				_elements = elements;
			}

			public override string ToString() => "<array index function>";

			public PortInfo[] Inputs { get; } =
			{
				new PortInfo {Name = "i", Type = NumberType.Instance}
			};

			public PortInfo[] Outputs { get; } = {
				new PortInfo{Name = "return", Type = AnyType.Instance}
			};

			private readonly IFunction[] _elements;

			public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
			{
				return this.CheckArguments(arguments, output, context) ?? Indexer.Create(arguments[0], _elements, context);
			}
		}

		private class Indexer : IFunction
		{
			public static IFunction Create(IFunction index, IFunction[] elements, CompilationContext info)
			{
				if (index.AsExpression(info) != null
					&& elements.All(e => e.AsExpression(info) != null))
				{
					return new Mux(index.AsExpression(info), elements.Select(e => e.AsExpression(info)));
				}

				if (elements.Any(e => e.IsLeaf()))
				{
					return NumberType.Instance;
				}

				return new Indexer(index, elements);
			}

			private Indexer(IFunction index, IFunction[] elements)
			{
				_index = index;
				_elements = elements;
			}

			public override string ToString() => "<array element>";
			public PortInfo[] Inputs => _elements[0].Inputs;
			public PortInfo[] Outputs => _elements[0].Outputs;
			private readonly IFunction _index;
			private readonly IFunction[] _elements;

			public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
			{
				return this.CheckArguments(arguments, output, context)
					?? Create(_index, _elements.Select(e => e.Call(arguments, output, context)).ToArray(), context);
			}
		}
	}
}