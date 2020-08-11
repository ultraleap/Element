using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using Element.AST;
using LinqExpression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    public static class CLRFunction
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
		    {Unary.Op.Cos, ((Func<double, double>)Math.Cos).Method},
		    {Unary.Op.ACos, ((Func<double, double>)Math.Acos).Method},
		    {Unary.Op.Tan, ((Func<double, double>)Math.Tan).Method},
		    {Unary.Op.ATan, ((Func<double, double>)Math.Atan).Method},
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
			public Dictionary<CachedInstruction, ParameterExpression> Cache;
			public Dictionary<InstructionGroup, LinqExpression[]> GroupCache;
			public Func<State, LinqExpression> ResolveState;
			//public List<float> StateValues;
			//public ParameterExpression StateArray;
			public Dictionary<Instruction, CachedInstruction> CSECache;
			//public Dictionary<ElementExpression, ElementExpression> ConstantCache;
		}

		private static LinqExpression Compile(Instruction value, CompilationData data)
		{
			data.Cache ??= new Dictionary<CachedInstruction, ParameterExpression>();
			data.GroupCache ??= new Dictionary<InstructionGroup, LinqExpression[]>();
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
				case CachedInstruction v:
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
						throw new Exception("Tried to compile a State expression outside of an InstructionGroup");
					}

					return data.ResolveState(s);
				case InstructionGroupElement groupElement:
					if (!data.GroupCache.TryGetValue(groupElement.Group, out var groupList))
					{
						data.GroupCache.Add(groupElement.Group, groupList = CompileGroup(groupElement.Group, data));
					}

					return groupList[groupElement.Index];
			}

			throw new Exception("Unknown expression " + value);
		}

		private static LinqExpression[] CompileGroup(InstructionGroup group, CompilationData data)
		{
			switch (group)
			{
				/*case Persist p:
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
					                 .ToArray();*/
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
					data.Cache = new Dictionary<CachedInstruction, ParameterExpression>(data.Cache);

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

		/// <summary>
		/// Replaces the inputs of a Function with expressions that map to a pre-allocated array.
		/// This is useful for applications where the exact inputs may be unknown or user-defined.
		/// </summary>
		/// <remarks>
		/// All the inputs to the function must be serializable and constant size (No dynamic lists).
		/// </remarks>
		/// <returns>
		/// The result of calling the given function with all its inputs and a pre-allocated argument array.
		/// The function inputs are mapped directly to the arrays contents.
		/// </returns>
		public static Result<(IValue CapturingValue, float[] CaptureArray)> SourceArgumentsFromSerializedArray(this IValue function, Context context) =>
			function.IsFunction()
				? function.InputPorts.Select(c => c.DefaultValue(context))
				          .BindEnumerable(defaultValues =>
				          {
					          var defaults = defaultValues as IValue[] ?? defaultValues.ToArray();
					          return defaults.Select(value => value.SerializedSize(context))
					                         .MapEnumerable(argSizes => (defaults, argSizes));
				          })
				          .Bind(tuple =>
				          {
					          var (defaultValues, argSizes) = tuple;
					          var totalSerializedSize = argSizes.Sum();

					          // Allocate the array and make an instructions for it
					          var array = new float[totalSerializedSize];
					          var arrayObjectExpr = LinqExpression.Constant(array);

					          return Enumerable.Range(0, totalSerializedSize)
					                           // For each array index, create an ArrayIndex expression
					                           .Select(i => NumberConverter.Instance
					                                                       .LinqToElement(LinqExpression.ArrayIndex(arrayObjectExpr, LinqExpression.Constant(i)), null!, context)
					                                                       .Cast<Instruction>(context))
					                           // Aggregate enumerable of results into single result
					                           .ToResultArray()
					                           .Bind(expressions =>
					                           {
						                           // Create a func for accessing the array items
						                           var flattenedValIdx = 0;
						                           Instruction NextValue() => expressions[flattenedValIdx++];
						                           return defaultValues.Select(v => v.Deserialize(NextValue, context))
						                                               .BindEnumerable(arguments => function.Call(arguments.ToArray(), context));
					                           })
					                           .Map(result => (result, array));
				          })
				: context.Trace(MessageCode.NotFunction, $"'{function}' is not a function, cannot source arguments");

		public static Result<TDelegate> Compile<TDelegate>(this IValue value, Context context, IBoundaryConverter? boundaryConverter = default)
			where TDelegate : Delegate =>
			Compile(value, context, typeof(TDelegate), boundaryConverter).Map(result => (TDelegate)result);

		private static Result<Delegate> Compile(IValue value, Context context, 
		                                 Type delegateType, IBoundaryConverter? boundaryConverter = default)
        {
	        boundaryConverter ??= new BoundaryConverter();

            // Check return type/single out parameter of delegate
            var method = delegateType.GetMethod(nameof(Action.Invoke));
            if (method == null) return context.Trace(MessageCode.InvalidBoundaryFunction, $"{delegateType} did not have invoke method");

            var delegateParameters = method.GetParameters();
            var delegateReturn = method.ReturnParameter;
            if (delegateParameters.Any(p => p.IsOut) || method.ReturnType == typeof(void))
            {
                return context.Trace(MessageCode.InvalidBoundaryFunction, $"{delegateType} cannot have out parameters and must have non-void return type");
            }
            
            // Create parameter expressions for input parameters and the functions return
            var parameterExpressions = delegateParameters.Select(p => LinqExpression.Parameter(p.ParameterType, p.Name)).ToArray();
            var returnExpression = LinqExpression.Parameter(delegateReturn.ParameterType, delegateReturn.Name);

            
            if (value.IsFunction() && value.InputPorts.Count != delegateParameters.Length)
            {
	            return context.Trace(MessageCode.InvalidBoundaryFunction, "Mismatch in number of parameters between delegate type and the function being compiled");
            }

            var resultBuilder = new ResultBuilder<Delegate>(context, default!);
            IValue outputExpression = value;
            // If input value is not a function we just try to use it directly
            if (value.IsFunction())
            {
	            var outputExpr = value.InputPorts
	                                  .Select((f, idx) => boundaryConverter.LinqToElement(parameterExpressions[idx], boundaryConverter, context))
	                                  .BindEnumerable(args => value.Call(args.ToArray(), context));
	            resultBuilder.Append(in outputExpr);
	            if (outputExpr.IsError) return resultBuilder.ToResult();
	            outputExpression = outputExpr.ResultOr(default!);
            }

            var data = new CompilationData
            {
                //StateArray = LinqExpression.Variable(typeof(float[])),
                //StateValues = new List<float>(),
                Statements = new List<LinqExpression>(),
                Variables = new List<ParameterExpression>(),
                Cache = new Dictionary<CachedInstruction, ParameterExpression>(),
                CSECache = new Dictionary<Instruction, CachedInstruction>(),
                //ConstantCache = new Dictionary<ElementExpression, ElementExpression>()
            };

            static bool IsPrimitiveElementType(Type t) => t == typeof(float) || t == typeof(bool);
            
            // Compile delegate
            var detectCircular = new Stack<IValue>();
            Result<LinqExpression> ConvertFunction(IValue value, Type outputType, Context context)
			{
				if (detectCircular.Count >= 1 && detectCircular.Peek() == value)
				{
					return context.Trace(MessageCode.RecursionNotAllowed, $"Circular dependency when compiling '{value}'");
				}

				// If this value is serializable then serialize and use it
				if (value.IsSerializable(context))
				{
					return value.Serialize(context)
					         .Bind(serialized => serialized.Count switch
					         {
						         1 when IsPrimitiveElementType(outputType) => Compile(serialized[0].Cache(data.CSECache), data),
						         _ => boundaryConverter.ElementToLinq(value, outputType, ConvertFunction, context)
					         });
				}

				// Else we try to use a boundary converter to convert to serializable instructions
				// TODO: Move circular checks to boundary converters
				detectCircular.Push(value);
				var retval = boundaryConverter.ElementToLinq(value, outputType, ConvertFunction, context);
				detectCircular.Pop();
				return retval;
			}

            return boundaryConverter.ElementToLinq(outputExpression, returnExpression.Type, ConvertFunction, context)
                                    .Map(convertedOutputExpression =>
                                    {
	                                    var outputAssign = LinqExpression.Assign(returnExpression, convertedOutputExpression);

	                                    data.Variables.Add(returnExpression);
	                                    data.Statements.Add(outputAssign);

	                                    // Put everything into a single code block, and wrap it in the Delegate
	                                    var fnBody = LinqExpression.Block(data.Variables, data.Statements);
	                                    return LinqExpression.Lambda(delegateType, fnBody, false, parameterExpressions).Compile();
                                    });
        }
    }
}