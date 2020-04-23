using Element.AST;

namespace Element
{
	using System.Linq;

	public static class Evaluator
	{
		/// <summary>
		/// Evaluate a function with the given serialized arguments, returning the outputs as an IFunction.
		/// </summary>
		public static IFunction Evaluate(this IFunction function, float[] argumentsSerialized, CompilationContext context)
		{
			var inputs = function.Inputs;
			var idx = 0;

			Expression NextValue() => new Constant(argumentsSerialized[idx++]);
			var arguments = inputs.Select(i => i.Type.Deserialize(NextValue, context)).ToArray();
			return function.Call(arguments, context);
		}

		/// <summary>
		/// Evaluate a function with the given serialized arguments, returning the outputs as a serialized array of floats.
		/// </summary>
		public static float[] EvaluateAndSerialize(this IFunction function, float[] argumentsSerialized, CompilationContext context) =>
			function.Evaluate(argumentsSerialized, context)
			        .Serialize(context)
			        .Select(e => ConstantFolding.Optimize(e))
			        .Select(v => ((Constant)v).Value)
			        .ToArray();
	}
}