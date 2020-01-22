namespace Element.CLR
{
	using System.Linq;
	using System.Linq.Expressions;
	using ElementExpression = Element.Expression;
	using LinqExpression = System.Linq.Expressions.Expression;
	using System.Collections.Generic;
	using System.Reflection;
	using System;
	using CLR;

	public static class CLRFunction
	{
		private static readonly Dictionary<Binary.Op, Func<LinqExpression, LinqExpression, BinaryExpression>> BinaryOps =
			new Dictionary<Binary.Op, Func<LinqExpression, LinqExpression, BinaryExpression>>
			{
				{Binary.Op.Add, LinqExpression.Add},
				{Binary.Op.Sub, LinqExpression.Subtract},
				{Binary.Op.Mul, LinqExpression.Multiply},
				{Binary.Op.Div, LinqExpression.Divide},
				{Binary.Op.Rem, LinqExpression.Modulo}
			};

		private static readonly Dictionary<Unary.Op, MethodInfo> MethodOps = new Dictionary<Unary.Op, MethodInfo>
		{
			{Unary.Op.Ln, ((Func<double, double>)Math.Log).Method},
			{Unary.Op.Sin, ((Func<double, double>)Math.Sin).Method},
			{Unary.Op.ASin, ((Func<double, double>)Math.Asin).Method},
		};

		private struct CompilationData
		{
			public List<LinqExpression> Statements;
			public List<ParameterExpression> Variables;
			public Dictionary<CachedExpression, ParameterExpression> Cache;
			public Dictionary<ExpressionGroup, LinqExpression[]> GroupCache;
			public Func<State, LinqExpression> ResolveState;
			public List<float> StateValues;
			public ParameterExpression StateArray;
			public Dictionary<ElementExpression, CachedExpression> CSECache;
			public Dictionary<ElementExpression, ElementExpression> ConstantCache;
		}

		private static LinqExpression Compile(ElementExpression value, CompilationData data)
		{
			data.Cache ??= new Dictionary<CachedExpression, ParameterExpression>();
			data.GroupCache ??= new Dictionary<ExpressionGroup, LinqExpression[]>();
			switch (value)
			{
				case ICLRExpression s:
					return s.Compile(e => Compile(e, data));
				case Constant c:
					return LinqExpression.Constant(c.Value);
				case Binary b:
					var ea = Compile(b.OpA, data);
					var eb = Compile(b.OpB, data);
					if (b.Operation == Binary.Op.Pow)
					{
						var o = LinqExpression.Call(typeof(Math).GetMethod(nameof(Math.Pow)),
							LinqExpression.Convert(ea, typeof(double)), LinqExpression.Convert(eb, typeof(double)));
						return LinqExpression.Convert(o, typeof(float));
					}

					if (b.Operation == Binary.Op.Atan2)
					{
						var o = LinqExpression.Call(typeof(Math).GetMethod(nameof(Math.Atan2)),
							LinqExpression.Convert(ea, typeof(double)), LinqExpression.Convert(eb, typeof(double)));
						return LinqExpression.Convert(o, typeof(float));
					}

					return BinaryOps[b.Operation](ea, eb);
				case Unary u:
					var input = Compile(u.Operand, data);
					var output = LinqExpression.Call(MethodOps[u.Operation], LinqExpression.Convert(input, typeof(double)));
					return LinqExpression.Convert(output, typeof(float));
				case CachedExpression v:
					if (!data.Cache.TryGetValue(v, out var varExpr))
					{
						var result = Compile(v.Value, data);
						data.Cache.Add(v, varExpr = LinqExpression.Parameter(result.Type));
						data.Statements.Add(LinqExpression.Assign(varExpr, result));
						data.Variables.Add(varExpr);
					}

					return varExpr;
				case Mux m:
					if (m.Operands.Count == 2)
					{
						return LinqExpression.Condition(
							LinqExpression.LessThan(Compile(m.Selector, data), LinqExpression.Constant(1f)),
							Compile(m.Operands[0], data), Compile(m.Operands[1], data));
					}

					var sel = LinqExpression.Convert(Compile(m.Selector, data), typeof(int));
					var clampedSel =
						LinqExpression.Condition(
							LinqExpression.GreaterThanOrEqual(sel, LinqExpression.Constant(m.Operands.Count)),
							LinqExpression.Constant(m.Operands.Count - 1), sel);
					var cases = m.Operands.Select(
						             (c, i) => LinqExpression.SwitchCase(Compile(c, data), LinqExpression.Constant(i)))
					             .ToArray();
					return LinqExpression.Switch(clampedSel, cases[0].Body, cases);
				case State s:
					if (data.ResolveState == null)
					{
						throw new Exception("Tried to compile a State expression outside of an ExpressionGroup");
					}

					return data.ResolveState(s);
				case ExpressionGroupElement groupElement:
					if (!data.GroupCache.TryGetValue(groupElement.Group, out var groupList))
					{
						data.GroupCache.Add(groupElement.Group, groupList = CompileGroup(groupElement.Group, data));
					}

					return groupList[groupElement.Index];
			}

			throw new Exception("Unknown expression " + value);
		}

		private static LinqExpression[] CompileGroup(ExpressionGroup group, CompilationData data)
		{
			switch (group)
			{
				case Persist p:
					foreach (var s in p.State)
					{
						if (!(s.InitialValue is Constant))
						{
							throw new Exception("Persist initial value is not constant");
						}

						data.StateValues.Add(((Constant)s.InitialValue).Value);
					}

					var assigns = new List<LinqExpression>();
					// TODO: Resolve parent scopes
					data.ResolveState = s => LinqExpression.ArrayAccess(data.StateArray, LinqExpression.Constant(s.Id));
					for (var i = 0; i < p.NewValue.Count; i++)
					{
						var newValueExpr = Compile(p.NewValue[i], data);
						assigns.Add(LinqExpression.Assign(
							LinqExpression.ArrayAccess(data.StateArray, LinqExpression.Constant(i)), newValueExpr));
					}

					data.Statements.AddRange(assigns);
					return Enumerable.Range(0, p.Size)
					                 .Select(i => LinqExpression.ArrayAccess(data.StateArray, LinqExpression.Constant(i)))
					                 .ToArray();
				case Loop l:
					var stateList = new List<ParameterExpression>();
					foreach (var s in l.State)
					{
						var initial = Compile(s.InitialValue, data);
						var variable = LinqExpression.Variable(initial.Type);
						data.Statements.Add(LinqExpression.Assign(variable, initial));
						data.Variables.Add(variable);
						stateList.Add(variable);
					}

					// TODO: Resolve parent scopes
					data.ResolveState = s => stateList[s.Id];
					var parentStatements = data.Statements;
					// Make a new cache that copies in the old one, but won't leak State expressions
					data.Cache = new Dictionary<CachedExpression, ParameterExpression>(data.Cache);

					// Create a new statements list to put in the loop body
					var s1 = data.Statements = new List<LinqExpression>();
					var condition = Compile(l.Condition, data);
					var s2 = data.Statements = new List<LinqExpression>();
					var newState = l.Body.Select(e => Compile(e, data)).ToArray();

					// Ensure that the entire state is only set at the end of the loop
					for (var i = 0; i < newState.Length; i++)
					{
						var s = newState[i];
						if (!(s is ParameterExpression))
						{
							var tmpVar = LinqExpression.Variable(s.Type);
							data.Variables.Add(tmpVar);
							s2.Add(LinqExpression.Assign(tmpVar, s));
							newState[i] = tmpVar;
						}
					}

					var breakLabel = LinqExpression.Label();
					var body = LinqExpression.Block(s1
					                             .Concat(new[]
					                             {
						                             LinqExpression.IfThen(
							                             LinqExpression.LessThan(condition, LinqExpression.Constant(1f)),
							                             LinqExpression.Break(breakLabel))
					                             })
					                             .Concat(s2)
					                             .Concat(newState.Select((e, i) =>
						                             LinqExpression.Assign(stateList[i], e))));
					parentStatements.Add(LinqExpression.Loop(body, breakLabel));
					return stateList.ToArray();
				default:
					throw new NotSupportedException();
			}
		}

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
			if (!CompilationContext.TryCreate(input, out var context)) return null;
			if (function == null) throw new ArgumentNullException(nameof(function));
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