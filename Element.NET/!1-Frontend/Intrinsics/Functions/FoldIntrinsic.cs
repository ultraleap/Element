using System;
using System.Linq;

namespace Element.AST
{
	public class FoldIntrinsic : IIntrinsic, IFunction
	{
		public IType Type => FunctionType.Instance;
		public string Location => "List.fold";
		public Port[] Inputs { get; } = {new Port(_atIdentifier, FunctionType.Annotation), new Port(_countIdentifier, NumType.Annotation)};
		public TypeAnnotation? Output => null;
		public IValue Call(IValue[] arguments, CompilationContext compilationContext)
		{
			var list = EvaluateArray(arguments[0] as IScope, compilationContext);
			var workingValue = arguments[1];
			var aggregator = (ICallable)arguments[2];
			return list.Aggregate(workingValue, (current, e) => aggregator.Call(new[] {current, e}, compilationContext));
		}


		private static readonly Identifier _atIdentifier = new Identifier("at");
		private static readonly Identifier _countIdentifier = new Identifier("count");

		/// <summary>
		/// Converts an Element List instance to a fixed-size list of values by evaluating each index.
		/// </summary>
		/// <returns>The evaluated array, or an empty array if there was an error</returns>
		private static IValue[]? EvaluateArray(IScope listInstance, CompilationContext context)
		{
			if (listInstance == null) throw new ArgumentNullException(nameof(listInstance));

			if (!(listInstance[_atIdentifier, false, context] is ICallable indexer))
			{
				context.LogError(8, $"Couldn't get List.'{_atIdentifier}' from '{listInstance}'. Is '{listInstance}' a List?");
				return null;
			}

			if (!(listInstance[_countIdentifier, false, context] is Literal count))
			{
				context.LogError(8, $"Couldn't get List.'{_countIdentifier}' from '{listInstance}'. Is '{listInstance}' a List?");
				return null;
			}

			/*var countExpr = listInstance.Call("count", context).AsExpression(context);
			if (countExpr != null) { countExpr = ConstantFolding.Optimize(countExpr); }

			var count = (countExpr as Constant)?.Value;
			if (!count.HasValue)
			{
				context.LogError(9999, $"{listInstance}'s count is not constant");
				return Array.Empty<IFunction>();
			}*/

			return Enumerable.Range(0, (int)count.Value)
			                 .Select(i => indexer.Call(new IValue[] {new Literal(i)}, context))
			                 .ToArray();
		}
	}
}