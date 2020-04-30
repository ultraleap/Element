using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using Element.AST;
using ElementExpression = Element.Expression;
using LinqExpression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    public static partial class CLRFunction
    {
	    private static readonly Dictionary<Unary.Op, Func<LinqExpression, UnaryExpression>> _linqUnaryOps =
		    new Dictionary<Unary.Op, Func<LinqExpression, UnaryExpression>>
		    {
			    {Unary.Op.Not, LinqExpression.Not}
		    };
	    
	    private static readonly Dictionary<Unary.Op, MethodInfo> _unaryMethodOps = new Dictionary<Unary.Op, MethodInfo>
	    {
		    {Unary.Op.Ln, ((Func<double, double>)Math.Log).Method},
		    {Unary.Op.Sin, ((Func<double, double>)Math.Sin).Method},
		    {Unary.Op.ASin, ((Func<double, double>)Math.Asin).Method},
	    };
	    
	    private static readonly Dictionary<Binary.Op, Func<LinqExpression, LinqExpression, BinaryExpression>> _linqBinaryArithmeticOps =
			new Dictionary<Binary.Op, Func<LinqExpression, LinqExpression, BinaryExpression>>
			{
				{Binary.Op.Add, LinqExpression.Add},
				{Binary.Op.Sub, LinqExpression.Subtract},
				{Binary.Op.Mul, LinqExpression.Multiply},
				{Binary.Op.Div, LinqExpression.Divide},
				{Binary.Op.Rem, LinqExpression.Modulo},
				{Binary.Op.Pow, LinqExpression.Power},
			};
	    
	    private static readonly Dictionary<Binary.Op, Func<LinqExpression, LinqExpression, BinaryExpression>> _linqBinaryComparisonOps =
			new Dictionary<Binary.Op, Func<LinqExpression, LinqExpression, BinaryExpression>>
			{
				{Binary.Op.Eq, LinqExpression.Equal},
				{Binary.Op.NEq, LinqExpression.NotEqual},
				{Binary.Op.Lt, LinqExpression.LessThan},
				{Binary.Op.LEq, LinqExpression.LessThanOrEqual},
				{Binary.Op.Gt, LinqExpression.GreaterThan},
				{Binary.Op.GEq, LinqExpression.GreaterThanOrEqual}
			};
	    
	    private static readonly Dictionary<Binary.Op, Func<LinqExpression, LinqExpression, BinaryExpression>> _linqBinaryLogicalOps =
			new Dictionary<Binary.Op, Func<LinqExpression, LinqExpression, BinaryExpression>>
			{
				{Binary.Op.And, LinqExpression.AndAlso},
				{Binary.Op.Or, LinqExpression.OrElse},
			};
	    
	    private static readonly Dictionary<Binary.Op, MethodInfo> _binaryMethodOps = new Dictionary<Binary.Op, MethodInfo>
		{
			{Binary.Op.Log, ((Func<double, double, double>)Math.Log).Method},
			{Binary.Op.Atan2, ((Func<double, double, double>)Math.Atan2).Method}
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
				case Unary u:
					var ua = Compile(u.Operand, data);
					if (_unaryMethodOps.TryGetValue(u.Operation, out var unaryMethod))
					{
						var methodExpr = LinqExpression.Call(unaryMethod, LinqExpression.Convert(ua, typeof(double)));
						return LinqExpression.Convert(methodExpr, typeof(float));
					}


					if (_linqUnaryOps.TryGetValue(u.Operation, out var linqUnaryOp))
					{
						return linqUnaryOp(ua);
					}

					break;
				case Binary b:
					var ba = Compile(b.OpA, data);
					var bb = Compile(b.OpB, data);

					if (_binaryMethodOps.TryGetValue(b.Operation, out var binaryMethod))
					{
						var methodExpr = LinqExpression.Call(binaryMethod,
						                                     LinqExpression.Convert(ba, typeof(double)),
						                                     LinqExpression.Convert(bb, typeof(double)));
						return LinqExpression.Convert(methodExpr, typeof(float));
					}

					if (_linqBinaryArithmeticOps.TryGetValue(b.Operation, out var linqBinaryOp))
					{
						return linqBinaryOp(ba, bb);
					}
					
					if (_linqBinaryComparisonOps.TryGetValue(b.Operation, out linqBinaryOp))
					{
						return LinqExpression.Condition(linqBinaryOp(ba, bb), LinqExpression.Constant(1f), LinqExpression.Constant(0f));
					}
					
					if (_linqBinaryLogicalOps.TryGetValue(b.Operation, out linqBinaryOp))
					{
						static LinqExpression ToBool(LinqExpression numExpr) => LinqExpression.Condition(LinqExpression.LessThan(numExpr, LinqExpression.Constant(1f)),
						                                                                                 LinqExpression.Constant(false),
						                                                                                 LinqExpression.Constant(true));
						return LinqExpression.Condition(linqBinaryOp(ToBool(ba), ToBool(bb)), LinqExpression.Constant(1f), LinqExpression.Constant(0f));
					}

					break;
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
		
		public static TDelegate Compile<TDelegate>(this SourceContext sourceContext, string functionExpression,
		                                           IBoundaryConverter boundaryConverter = default)
			where TDelegate : Delegate? =>
			sourceContext.EvaluateExpression(functionExpression, out _) is { } val
				? (TDelegate)sourceContext.Compile(val, typeof(TDelegate), boundaryConverter)
				: null;
		
		public static TDelegate Compile<TDelegate>(this SourceContext sourceContext, IValue value,
		                                            IBoundaryConverter boundaryConverter = default)
			where TDelegate : Delegate? =>
			(TDelegate)sourceContext.Compile(value, typeof(TDelegate), boundaryConverter);

		private static Delegate? Compile(this SourceContext sourceContext, IValue value,
		                                 Type delegateType, IBoundaryConverter boundaryConverter = default)
        {
            if (sourceContext == null) throw new ArgumentNullException(nameof(sourceContext));
            if (value == null) throw new ArgumentNullException(nameof(value));
            if (delegateType == null) throw new ArgumentNullException(nameof(delegateType));

            sourceContext.MakeCompilationContext(out var context);
            boundaryConverter ??= new BoundaryConverter();

            // Check return type/single out parameter of delegate
            var method = delegateType.GetMethod(nameof(Action.Invoke));
            if (method == null)
            {
	            context.LogError(10, $"{delegateType} did not have invoke method");
	            return null;
            }
            
            var delegateParameters = method.GetParameters();
            var delegateReturn = method.ReturnParameter;
            if (delegateParameters.Any(p => p.IsOut) || method.ReturnType == typeof(void))
            {
                context.LogError(10, $"{delegateType} cannot have out parameters and must have non-void return type");
                return null;
            }

            // Create parameter expressions
            var parameterExpressions = delegateParameters.Select(p => LinqExpression.Parameter(p.ParameterType, p.Name)).ToArray();
            var returnExpression = LinqExpression.Parameter(delegateReturn.ParameterType, delegateReturn.Name);

            var reducedExpression = value switch
            {
	            IFunctionSignature functionSignature => functionSignature.ResolveCall(
		            functionSignature.Inputs.Select(f =>
		                             {
			                             ParameterExpression p;
			                             return (p = Array.Find(parameterExpressions, i => i.Name == f.Identifier)) == null
				                                    ? context.LogError(10, $"Unable to bind {functionSignature}'s input {f} - could not find matching parameter name on delegate")
				                                    : boundaryConverter.LinqToElement(p, boundaryConverter, context);
		                             }).ToArray(), false, context),
	            Expression expr => expr,
	            _ => context.LogError(3, $"'{value}' is not compilable")
            };

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
            
            // Compile delegate
            var detectCircular = new Stack<IValue>();
            LinqExpression? ConvertFunction(IValue value, Type outputType, CompilationContext context)
			{
				switch (value)
				{
					case CompilationErr _: return null;
					case IConstraint c:
						context.LogError(3, $"Cannot compile a <{c.Type}>: {c}");
						return null;
				}

				if (detectCircular.Count >= 1 && detectCircular.Peek() == value)
				{
					context.LogError(11, $"Circular dependency when compiling '{value}'");
					return null;
				}

				if(value.TrySerialize(out Expression[] expressions, context))
				{
					var expr = expressions.Single();
					expr = ConstantFolding.Optimize(expr, data.ConstantCache);
					expr = CommonSubexpressionExtraction.OptimizeSingle(data.CSECache, expr);
					value = expr;
					return outputType == typeof(float)
						? Compile(expr, data)
						: boundaryConverter.ElementToLinq(value, outputType, ConvertFunction, context);
				}

				if (outputType == typeof(float))
				{
					throw new InternalCompilerException($"Could not compile {value} - output type is float but {value} is not an expression.");
				}

				detectCircular.Push(value);
				var retval = boundaryConverter.ElementToLinq(value, outputType, ConvertFunction, context);
				detectCircular.Pop();
				return retval;
			}

            var outputExpression = boundaryConverter.ElementToLinq(reducedExpression, returnExpression.Type, ConvertFunction, context);
            var outputAssign = LinqExpression.Assign(returnExpression, outputExpression);
            
	        data.Variables.Add(returnExpression);
	        data.Statements.Add(outputAssign);

			// Put everything into a single code block, and wrap it in the Delegate
			var fnBody = LinqExpression.Block(data.Variables, data.Statements);
			return LinqExpression.Lambda(delegateType, fnBody, false, parameterExpressions).Compile();
        }
    }
}