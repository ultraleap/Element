namespace Element
{
	using System;
	using System.Linq;

	public class FoldIntrinsic : INamedFunction
	{
		public string Name => "fold";

		public PortInfo[] Inputs { get; } =
		{
			new PortInfo {Name = "foldFunction", Type = FunctionType.Instance},
			new PortInfo {Name = "array", Type = AnyType.Instance},
			new PortInfo {Name = "initial", Type = AnyType.Instance}
		};

		public PortInfo[] Outputs { get; } =
		{
			new PortInfo {Name = "return", Type = AnyType.Instance}
		};

		public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
		{
			if (this.CheckArguments(arguments, output, context) != null)
			{
				return Error.Instance;
			}

			var aggregator = arguments[0];
			var array = EvaluateArray(arguments[1], context);
			var workingValue = arguments[2];
			// TODO: For type resolution, we could stop this if the interface didn't change in a cycle
			foreach (var e in array)
			{
				workingValue = aggregator.Call(new[] {workingValue, e}, context);
			}

			return workingValue;
		}

		/// <summary>
		/// Converts an Element Array instance (with count and index members) to
		/// a fixed-size list of Functions by evaluating each index.
		/// </summary>
		/// <returns>The evaluated array, or an empty array if there was an error</returns>
		private static IFunction[] EvaluateArray(IFunction function, CompilationContext context)
		{
			if (function == null) throw new ArgumentNullException(nameof(function));
			if (function.Inputs?.Length != 0)
			{
				context.LogError(9999, $"{function} needs to be an array, but it has inputs");
				return Array.Empty<IFunction>();
			}

			var countExpr = function.Call("count", context).AsExpression(context);
			if (countExpr != null) { countExpr = ConstantFolding.Optimize(countExpr); }

			var count = (countExpr as Constant)?.Value;
			if (!count.HasValue)
			{
				context.LogError(9999, $"{function}'s count is not constant");
				return Array.Empty<IFunction>();
			}

			var index = function.Call("index", context);
			return Enumerable.Range(0, (int)count.Value)
			                 .Select(i => index.Call(new[] {new Constant(i)}, context))
			                 .ToArray();
		}
	}
}