namespace Element
{
	using System;
	using System.Linq;
	using System.Collections.Generic;

	/// <summary>
	/// Creates an anonymous struct instance with special indexing rules.
	/// When indexed, the passed function will be called using the remaining parameters with the indexing expression.
	/// memberwise(function, a, b, ...) -> {function(a, b, ...), ...}
	/// memberwise(function, a, b, ...).expr -> function(a.expr, b.expr, ...)
	/// </summary>
	internal class MemberwiseIntrinsic : INamedFunction
	{
		public string Name => "memberwise";
		public PortInfo[] Inputs => null;

		public PortInfo[] Outputs { get; } =
		{
			new PortInfo {Name = "return", Type = AnyType.Instance}
		};

		public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
		{
			if (arguments.Length < 2)
			{
				return context.LogError(6, "memberwise requires at least 2 arguments");
			}

			if (arguments.Any(a => a is AnyType))
			{
				return AnyType.Instance;
			}

			if (arguments.Skip(1).Any(a => a.Inputs?.Length != 0))
			{
				return context.LogError(14, "one or more memberwise arguments are functions");
			}

			if (arguments.Skip(1).Any(a => a.Outputs == null))
			{
				return context.LogError(14, "one or more memberwise arguments is non-introspectable");
			}

			// TODO: Check all have same input signature?
			return new MapFunction(arguments[0], arguments.Skip(1).ToArray());
		}

		// TODO: Unify this with Indexer?
		private class MapFunction : IFunction
		{
			public MapFunction(IFunction function, IFunction[] elements)
			{
				_function = function;
				_elements = elements;
				Outputs = elements.Select(e => e.Outputs)
				                  .Cast<IEnumerable<PortInfo>>()
				                  .Aggregate((a, b) => a.Intersect(b))
				                  .ToArray();
			}

			private readonly IFunction _function;
			private readonly IFunction[] _elements;
			public override string ToString() => "<memberwise result>";

			public PortInfo[] Inputs => Array.Empty<PortInfo>();
			public PortInfo[] Outputs { get; }

			private static readonly Func<IFunction, bool> isExpression = e => e.IsLeaf();

			public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context) =>
				_elements.All(isExpression) && output == "return"
					? _function.Call(_elements, context)
					: _function.Call(_elements.Select(e => e.Call(arguments, output, context)).ToArray(), context);
		}
	}
}