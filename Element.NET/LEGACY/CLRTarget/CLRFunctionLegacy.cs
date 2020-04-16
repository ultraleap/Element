using Element.AST;

namespace Element.CLR
{
	using System.Linq;
	using System.Linq.Expressions;
	using ElementExpression = Element.Expression;
	using LinqExpression = System.Linq.Expressions.Expression;
	using System.Collections.Generic;
	using System.Reflection;
	using System;

	public static partial class CLRFunction
	{
		public static TDelegate Compile<TDelegate>(this IFunction function,
		                                           CompilationInput input,
		                                           ICLRBoundaryMap boundaryMap = null) where TDelegate : Delegate? =>
			(TDelegate)function.Compile(typeof(TDelegate), input, boundaryMap);

		/// <summary>
		/// Converts an Element function to a CLR function by matching similarly-named parameters
		/// </summary>
		/// <param name="function"></param>
		/// <param name="delegateType"></param>
		/// <param name="input"></param>
		/// <param name="boundaryMap">Maps CLR types to Element types and vice-versa</param>
		/// <returns>Compiled delegate or null if failure</returns>
		public static Delegate? Compile(this IFunction function, Type delegateType,
			CompilationInput input,
			ICLRBoundaryMap boundaryMap = default)
		{
			if (!SourceContext.TryCreate(input, out var sourceContext)) return null;
			if (function == null) throw new ArgumentNullException(nameof(function));

			sourceContext.MakeCompilationContext(out var context);
			if (function.Inputs == null || function.Outputs == null)
			{
				context.LogError(3, $"{function} cannot be compiled as it has no input/output ports defined");
				return null;
			}

			// Get the target delegate's inputs and outputs
			var method = delegateType.GetMethod(nameof(Action.Invoke));
			var fnParams = method.GetParameters();
			var fnOutputs = fnParams.Where(p => p.IsOut).ToArray();
			var usingReturnParameter = fnOutputs.Length == 0;
			if (usingReturnParameter)
			{
				fnOutputs = new[] {method.ReturnParameter};
				fnParams = fnParams.Concat(fnOutputs).ToArray();
			}

			// Read as "if method defines a non-void return type and we aren't using return type"
			if (method.ReturnType != typeof(void) != usingReturnParameter)
			{
				context.LogError(10, $"{delegateType} must have either out parameters or a non-void return type and cannot have both");
				return null;
			}

			boundaryMap ??= new RootCLRBoundaryMap();

			// Create an ordered list of ParameterExpressions that matches the delegate's signature
			var paramExpressions = fnParams.Select(p => LinqExpression.Parameter(p.ParameterType, p.Name)).ToArray();

			// Create a list of Element inputs by matching up ports to parameters
			var arguments = function.Inputs.Select(f =>
			                        {
				                        ParameterExpression p;
				                        return (p = Array.Find(paramExpressions, i => i.Name == f.Name)) == null
					                        ? context.LogError(10, $"Unable to bind {function}'s input {f} - could not find matching parameter name on delegate")
					                        : boundaryMap.ToInput(p, boundaryMap, context);
			                        })
			                        .ToArray();

			// Call the function with arguments to generate output functions
			var output = function.Call(arguments, context);

			IFunction[] outputFunctions;

			if (usingReturnParameter)
			{
				// If we're returning a single object, push the entire output structure
				outputFunctions = new[] {output};
			}
			else
			{
				// Create a list of Element outputs by matching up ports to parameters
				outputFunctions = fnOutputs
				                  .Select(f =>
				                  {
					                  var port = output.Outputs.FirstOrDefault(p => p.Name == f.Name);
					                  return string.IsNullOrEmpty(port.Name)
						                         ? context.LogError(10, $"Unable to bind {delegateType}'s output {f} - could not find matching ")
						                         : output.Call(port.Name, context);
				                  })
				                  .ToArray();
			}

			// Compile the generic expressions to Linq.Expressions, assigning them to the output parameters
			var data = new CompilationData
			{
				StateArray = LinqExpression.Variable(typeof(float[])),
				StateValues = new List<float>(),
				Statements = new List<LinqExpression>(),
				Variables = new List<ParameterExpression>(),
				Cache = new Dictionary<CachedExpression, ParameterExpression>(),
				CSECache = new Dictionary<ElementExpression, CachedExpression>(),
				ConstantCache = new Dictionary<ElementExpression, ElementExpression>()
			};

			var detectCircular = new Stack<IFunction>();
			LinqExpression? CompileFunction(IFunction fn, Type outputType, CompilationContext context)
			{
				switch (fn)
				{
					case CompilationError _: return null;
					case IType _:
						context.LogError(3, "Cannot compile a type");
						return null;
				}

				if (detectCircular.Count >= 1 && detectCircular.Peek() == fn)
				{
					context.LogError(11, $"Circular dependency when compiling '{fn}'");
					return null;
				}

				var expr = fn.AsExpression(context);
				if (expr != null)
				{
					expr = ConstantFolding.Optimize(expr, data.ConstantCache);
					expr = CommonSubexpressionExtraction.OptimizeSingle(data.CSECache, expr);
					fn = expr;
					return outputType == typeof(float)
						? Compile(expr, data)
						: boundaryMap.FromOutput(fn, outputType, CompileFunction, context);
				}

				if (outputType == typeof(float))
				{
					throw new InternalCompilerException($"Could not compile {fn} - output type is Single but {fn} is not an expression.");
				}

				detectCircular.Push(fn);
				var retval = boundaryMap.FromOutput(fn, outputType, CompileFunction, context);
				detectCircular.Pop();
				return retval;
			}

			var finalOutputs = outputFunctions.Select((func, i) =>
			                                  {
				                                  var pe = paramExpressions.First(p => p.Name == fnOutputs[i].Name);
				                                  var expr = boundaryMap.FromOutput(func, pe.Type, CompileFunction,
					                                  context);
				                                  return LinqExpression.Assign(pe, expr);
			                                  })
			                                  .Concat(usingReturnParameter
				                                  ? new LinqExpression[] {paramExpressions.Last()}
				                                  : Array.Empty<LinqExpression>())
			                                  .ToArray();

			if (usingReturnParameter)
			{
				data.Variables.Add(paramExpressions.Last());
			}

			// Set up the persistent storage
			if (data.StateValues.Count > 0)
			{
				data.Variables.Add(data.StateArray);
				data.Statements.Insert(0,
					LinqExpression.Assign(data.StateArray, LinqExpression.Constant(data.StateValues.ToArray())));
			}

			// Put everything into a single code block, and wrap it in the Delegate
			var fnBody = LinqExpression.Block(data.Variables, data.Statements.Concat(finalOutputs));
			//Console.WriteLine(fnBody.GetDebugView());
			return LinqExpression.Lambda(delegateType, fnBody, false,
					paramExpressions.Take(
						paramExpressions.Length - (usingReturnParameter ? 1 : 0)))
				.Compile();
		}

		public static string GetDebugView(this LinqExpression exp)
		{
			if (exp == null)
				return null;

			var propertyInfo =
				typeof(LinqExpression).GetProperty("DebugView", BindingFlags.Instance | BindingFlags.NonPublic);
			return propertyInfo.GetValue(exp) as string;
		}

		/// <summary>
		/// Replaces the inputs of a Function with expressions that map to a pre-allocated array.
		/// This is useful for applications where the exact inputs may be unknown or user-defined.
		/// </summary>
		/// <remarks>
		/// Naturally, all the inputs to the function must be serializable.
		/// </remarks>
		/// <param name="function"></param>
		/// <param name="array">The resultant pre-allocated array. The function inputs are mapped directly to its contents.</param>
		/// <returns>The result of calling `function` with all its inputs, or Abort where there was an error.</returns>
		public static IFunction ProvideConfiguration(this IFunction function, out float[] array, CompilationContext context)
		{
			var inputs = function.Inputs;
			var myCount = 0;
			// First count the total size so we can pre-allocate an array
			foreach (var i in inputs)
			{
				// The return value doesn't matter here; we just want the size
				i.Type.Deserialize(() =>
				{
					myCount++;
					return null;
				}, context);
			}

			// Now allocate the array, and make an argument list that uses it as data
			array = new float[myCount];
			var arrayExpr = LinqExpression.Constant(array);
			var idx = 0;

			ElementExpression NextValue() =>
				(ElementExpression)SingleInput.Instance.ToInput(LinqExpression.ArrayIndex(arrayExpr, LinqExpression.Constant(idx++)), null, context);

            var arguments = inputs.Select(i => i.Type.Deserialize(NextValue, context)).ToArray();
			return function.Call(arguments, context);
		}
	}
}