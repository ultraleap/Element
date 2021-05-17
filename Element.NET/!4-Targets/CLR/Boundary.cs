using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using Element.AST;
using ResultNET;
using LinqExpression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
	public static class Boundary
	{
		private static readonly Dictionary<Unary.Op, MethodInfo> _unaryMethodOps = new Dictionary<Unary.Op, MethodInfo>
	    {
		    {Unary.Op.Ln, ((Func<double, double>)Math.Log).Method},
		    {Unary.Op.Sin, ((Func<double, double>)Math.Sin).Method},
		    {Unary.Op.ASin, ((Func<double, double>)Math.Asin).Method},
		    {Unary.Op.Cos, ((Func<double, double>)Math.Cos).Method},
		    {Unary.Op.ACos, ((Func<double, double>)Math.Acos).Method},
		    {Unary.Op.Tan, ((Func<double, double>)Math.Tan).Method},
		    {Unary.Op.ATan, ((Func<double, double>)Math.Atan).Method},
		    {Unary.Op.Floor, ((Func<double, double>)Math.Floor).Method},
		    {Unary.Op.Ceil, ((Func<double, double>)Math.Ceiling).Method},
		    {Unary.Op.Abs, ((Func<float, float>)Math.Abs).Method},
	    };
	    
	    private static readonly Dictionary<Binary.Op, MethodInfo> _binaryMethodOps = new Dictionary<Binary.Op, MethodInfo>
	    {
		    {Binary.Op.Log, ((Func<double, double, double>)Math.Log).Method},
		    {Binary.Op.Atan2, ((Func<double, double, double>)Math.Atan2).Method},
		    {Binary.Op.Max, ((Func<float, float, float>)Math.Max).Method},
		    {Binary.Op.Min, ((Func<float, float, float>)Math.Min).Method},
	    };
	    
	    private static readonly Dictionary<Unary.Op, Func<LinqExpression, UnaryExpression>> _unaryLinqOps =
		    new Dictionary<Unary.Op, Func<LinqExpression, UnaryExpression>>
		    {
			    {Unary.Op.Not, LinqExpression.Not},
		    };

	    private static readonly Dictionary<Unary.Op, (Type In, Type Out)> _unaryLinqSignatures =
		    new Dictionary<Unary.Op, (Type In, Type Out)>
		    {
			    {Unary.Op.Not, (typeof(bool), typeof(bool))}
		    };

	    private static readonly Dictionary<Binary.Op, Func<LinqExpression, LinqExpression, BinaryExpression>> _binaryLinqOps =
			new Dictionary<Binary.Op, Func<LinqExpression, LinqExpression, BinaryExpression>>
			{
				// Float ops
				{Binary.Op.Add, LinqExpression.Add},
				{Binary.Op.Sub, LinqExpression.Subtract},
				{Binary.Op.Mul, LinqExpression.Multiply},
				{Binary.Op.Div, LinqExpression.Divide},
				{Binary.Op.Rem, LinqExpression.Modulo},
				
				// Double ops
				{Binary.Op.Pow, LinqExpression.Power},
				
				// Float comparison ops
				{Binary.Op.Eq, LinqExpression.Equal},
				{Binary.Op.NEq, LinqExpression.NotEqual},
				{Binary.Op.Lt, LinqExpression.LessThan},
				{Binary.Op.LEq, LinqExpression.LessThanOrEqual},
				{Binary.Op.Gt, LinqExpression.GreaterThan},
				{Binary.Op.GEq, LinqExpression.GreaterThanOrEqual},
				
				// Logical ops
				{Binary.Op.And, LinqExpression.AndAlso},
				{Binary.Op.Or, LinqExpression.OrElse},
			};
	    
	    private static readonly Dictionary<Binary.Op, (Type InA, Type InB, Type Out)> _binaryLinqSignatures =
		    new Dictionary<Binary.Op, (Type InA, Type InB, Type Out)>
		    {
			    // Float ops
			    {Binary.Op.Add, (typeof(float), typeof(float), typeof(float))},
			    {Binary.Op.Sub, (typeof(float), typeof(float), typeof(float))},
			    {Binary.Op.Mul, (typeof(float), typeof(float), typeof(float))},
			    {Binary.Op.Div, (typeof(float), typeof(float), typeof(float))},
			    {Binary.Op.Rem, (typeof(float), typeof(float), typeof(float))},
			    
			    // Double ops
			    {Binary.Op.Pow, (typeof(double), typeof(double), typeof(double))},
			    
			    // Float comparison ops
			    {Binary.Op.Eq, (typeof(float), typeof(float), typeof(bool))},
			    {Binary.Op.NEq, (typeof(float), typeof(float), typeof(bool))},
			    {Binary.Op.Lt, (typeof(float), typeof(float), typeof(bool))},
			    {Binary.Op.LEq, (typeof(float), typeof(float), typeof(bool))},
			    {Binary.Op.Gt, (typeof(float), typeof(float), typeof(bool))},
			    {Binary.Op.GEq, (typeof(float), typeof(float), typeof(bool))},
			    
			    // Logical ops
			    {Binary.Op.And, (typeof(bool), typeof(bool), typeof(bool))},
			    {Binary.Op.Or, (typeof(bool), typeof(bool), typeof(bool))},
		    };

	    private static readonly Dictionary<(Type From, Type To), Func<LinqExpression, LinqExpression>> _conversionFunctions =
		    new Dictionary<(Type From, Type To), Func<LinqExpression, LinqExpression>>
		    {
			    {(typeof(float), typeof(bool)), num => LinqExpression.Condition(LinqExpression.GreaterThan(num, LinqExpression.Constant(0f)), LinqExpression.Constant(true), LinqExpression.Constant(false))},
			    {(typeof(bool), typeof(float)), boolean => LinqExpression.Condition(boolean, LinqExpression.Constant(1f), LinqExpression.Constant(0f))},
			    {(typeof(float), typeof(double)), expression => LinqExpression.Convert(expression, typeof(double))},
			    {(typeof(double), typeof(float)), expression => LinqExpression.Convert(expression, typeof(float))}
		    };
	    
	    private struct CompilationData
		{
			public List<LinqExpression> Statements;
			public List<ParameterExpression> Variables;
			public Dictionary<CachedInstruction, ParameterExpression> Cache;
			public Dictionary<InstructionGroup, LinqExpression[]> GroupCache;
			public Func<State, LinqExpression>? ResolveState;
		}

		private static LinqExpression CompileInstruction(Instruction value, CompilationData data, Context context)
		{
			static LinqExpression ConvertExpressionType(LinqExpression expr, Type targetType) =>
				expr.Type == targetType
					? expr
					: _conversionFunctions.TryGetValue((expr.Type, targetType), out var convert)
						? convert(expr)
						: throw new NotSupportedException($"Conversion not defined from '{expr}' to '{targetType}'");

			static LinqExpression ConvertOutputExpressionType(LinqExpression expr, Type targetType)
			{
				var isDouble = expr.Type == typeof(double) && targetType == typeof(double);
				var converterExists = _conversionFunctions.TryGetValue((expr.Type, typeof(float)), out var forceConvert);
				
				return (isDouble, converterExists) switch
				{
					(true, true) => forceConvert(expr),
					(_, false) => ConvertExpressionType(expr, targetType),
					_ => throw new NotSupportedException($"Conversion not defined from '{expr}' to '{targetType}'")
				};
			}

			switch (value)
			{
				case ILinqExpression linqExpression:
					return linqExpression.Expression;
				case Constant c:
					return context.ElementToClr(c.StructImplementation)
					              .Match((type, messages) => ConvertExpressionType(LinqExpression.Constant(c.Value), type),
					                     // TODO: Return error instead of throwing
					                     messages => throw new InternalCompilerException($"No CLR type mapped for '{c.StructImplementation}'"));
				case Cast c:
					return context.ElementToClr(c.StructImplementation)
					              .Match((type, messages) => ConvertExpressionType(CompileInstruction(c.Instruction, data, context), type),
					                     // TODO: Return error instead of throwing
					                     messages => throw new InternalCompilerException($"No CLR type mapped for '{c.StructImplementation}'"));
				case Unary u:
					var ua = CompileInstruction(u.Operand, data, context);
					
					if (_unaryLinqOps.TryGetValue(u.Operation, out var linqUnaryOp))
					{
						return _unaryLinqSignatures.TryGetValue(u.Operation, out var unaryLinqOpSignature)
							       ? ConvertExpressionType(linqUnaryOp(ConvertExpressionType(ua, unaryLinqOpSignature.In)), unaryLinqOpSignature.Out)
							       : throw new NotImplementedException($"'{u.Operation}' is implemented as a linq expression but signature is not known");
					}
					
					if (_unaryMethodOps.TryGetValue(u.Operation, out var unaryMethod))
					{
						return ConvertOutputExpressionType(LinqExpression.Call(unaryMethod, ConvertExpressionType(ua, unaryMethod.GetParameters()[0].ParameterType)),
						                             unaryMethod.ReturnParameter!.ParameterType);
					}

					break;
				case Binary b:
					var ba = CompileInstruction(b.OpA, data, context);
					var bb = CompileInstruction(b.OpB, data, context);
					
					if (_binaryLinqOps.TryGetValue(b.Operation, out var linqBinaryOp))
					{
						return _binaryLinqSignatures.TryGetValue(b.Operation, out var binaryLinqOpSignature)
							       ? ConvertExpressionType(linqBinaryOp(ConvertExpressionType(ba, binaryLinqOpSignature.InA), ConvertExpressionType(bb, binaryLinqOpSignature.InB)), binaryLinqOpSignature.Out)
							       : throw new NotImplementedException($"'{b.Operation}' is implemented as a linq expression but signature is not known");
					}

					if (_binaryMethodOps.TryGetValue(b.Operation, out var binaryMethod))
					{
						return ConvertOutputExpressionType(LinqExpression.Call(binaryMethod,
						                                                 ConvertExpressionType(ba, binaryMethod.GetParameters()[0].ParameterType),
						                                                 ConvertExpressionType(bb, binaryMethod.GetParameters()[1].ParameterType)),
						                             binaryMethod.ReturnParameter!.ParameterType);
					}
					
					break;
				case CachedInstruction v:
					if (!data.Cache.TryGetValue(v, out var varExpr))
					{
						var result = CompileInstruction(v.Value, data, context);
						data.Cache.Add(v, varExpr = LinqExpression.Parameter(result.Type));
						data.Statements.Add(LinqExpression.Assign(varExpr, result));
						data.Variables.Add(varExpr);
					}

					return varExpr;
				case Switch m:
					
					// TODO: Don't clamp access to operands, use runtime error (nothing type? NaN?) instead!

					if (m.Operands.Count == 2)
					{
						var sa = CompileInstruction(m.Operands[1], data, context);
						var sb = CompileInstruction(m.Operands[0], data, context);

						return LinqExpression.Condition(ConvertExpressionType(CompileInstruction(m.Selector, data, context), typeof(bool)),
							sa,
							sb);
					}

					var sel = LinqExpression.Convert(CompileInstruction(m.Selector, data, context), typeof(int));
					var clampedSel =
						LinqExpression.Condition(
							LinqExpression.GreaterThanOrEqual(sel, LinqExpression.Constant(m.Operands.Count)),
							LinqExpression.Constant(m.Operands.Count - 1), sel);
					var cases = m.Operands.Select(
						             (c, i) => LinqExpression.SwitchCase(CompileInstruction(c, data, context), LinqExpression.Constant(i)))
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
						data.GroupCache.Add(groupElement.Group, groupList = CompileGroup(groupElement.Group, data, context));
					}

					return groupList[groupElement.Index];
			}

			throw new Exception("Unknown expression " + value);
		}

		private static LinqExpression[] CompileGroup(InstructionGroup group, CompilationData data, Context context)
		{
			switch (group)
			{
				case Loop l:
					var stateList = new List<ParameterExpression>();
					foreach (var s in l.State)
					{
						var initial = CompileInstruction(s.InitialValue, data, context);
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
					var condition = CompileInstruction(l.Condition, data, context);
					var s2 = data.Statements = new List<LinqExpression>();
					var newState = l.Body.Select(e => CompileInstruction(e, data, context)).ToArray();

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
					var body = LinqExpression.Block(s1.Concat(new[]
					                                  {
						                                  LinqExpression.IfThen(LinqExpression.IsFalse(condition),
						                                                        LinqExpression.Break(breakLabel))
					                                  })
					                                  .Concat(s2)
					                                  .Concat(newState.Select((e, i) => LinqExpression.Assign(stateList[i], e))));
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
		public static Result<(IValue CapturingValue, float[] CaptureArray)> SourceArgumentsFromSerializedArray(this IValue function, Context context)
		{
			if (!function.IsCallable(context)) return context.Trace(EleMessageCode.NotFunction, $"'{function}' is not a function, cannot source arguments");
			
			return function.SerializeAllInputPortDefaults(context)
			               .Bind(tuple =>
			               {
				               var (defaultValues, allDefaultsSerialized) = tuple;

				               // Allocate the array and make an instructions for it
				               var array = new float[allDefaultsSerialized.Length];
				               var arrayObjectExpr = LinqExpression.Constant(array);

				               Result<Instruction> SourceFromArrayIndex(Instruction instruction, int i) =>
					               context.ElementToClr(instruction.StructImplementation)
					                      .Bind(instructionClrType =>
					                      {
						                      Result<Instruction> GenerateNumberInstruction(LinqExpression expr) => NumberConverter.Instance
						                                                                                                           .LinqToElement(expr, context)
						                                                                                                           .Cast<Instruction>();
						                      var arrayIndexExpr = LinqExpression.ArrayIndex(arrayObjectExpr, LinqExpression.Constant(i));
						                      return GenerateNumberInstruction(instructionClrType == typeof(float)
							                                                       ? arrayIndexExpr
							                                                       : _conversionFunctions[(typeof(float), instructionClrType)](arrayIndexExpr));
					                      });

				               // Go over all the individual instructions (single leaf values) and source them from the array
				               Result<IValue> DeserializeArraySourcedInstructionsToObjectModel(Instruction[] instructions)
				               {
					               // Create a func for accessing the array items individually for deserializing
					               var flattenedValIdx = 0;
					               Instruction NextValue() => instructions[flattenedValIdx++];

					               // Deserialize the array-sourced instructions into element object model
					               // Then call the function to compile it further now that all the arguments are sourced from the array
					               return defaultValues.Select(v => v.Deserialize(NextValue, context))
					                                   .BindEnumerable(arguments => function.Call(arguments.ToArray(), context));
				               }

				               return allDefaultsSerialized.Select(SourceFromArrayIndex)
				                                           .ToResultArray()
				                                           .Bind(DeserializeArraySourcedInstructionsToObjectModel)
				                                           .Map(result => (result, array));
			               });
		}

		public static Result<TDelegate> Compile<TDelegate>(this IValue value, Context boundaryContext)
			where TDelegate : Delegate =>
			Compile(value, typeof(TDelegate), boundaryContext).Map(result => (TDelegate)result);
		
		private delegate TInstance InstanceDelegate<out TInstance>();

		/// <summary>
		/// Compile an Element IValue instance into a CLR type instance.
		/// The type of the Element IValue must be a valid boundary type with a defined CLR Type mapping in the BoundaryMap.
		/// </summary>
		public static Result<TInstance> CompileInstance<TInstance>(this IValue value, Context context) =>
			value.Compile<InstanceDelegate<TInstance>>(context).Map(fn => fn());

		/// <summary>
		/// Compile a delegate dynamically using an Element value.
		/// The compiled delegate's signature is based on the Element value's input port types and return type (which all must be valid boundary types).
		/// </summary>
		public static Result<Delegate> CompileDynamic(this IValue value, Context context)
		{
			Result<Struct> ConstraintToStruct(IValue constraint) => constraint.InnerIs<Struct>(out var s)
				                                                        ? new Result<Struct>(s)
				                                                        : context.Trace(EleMessageCode.InvalidBoundaryFunction, $"'{constraint}' is not a struct - all top-level function ports must be serializable struct types");

			var inputStructs = value.HasInputs()
				                   ? value.InputPorts.Select(p => ConstraintToStruct(p.ResolvedConstraint)).ToResultArray()
				                   : Array.Empty<Struct>();
			
			
			
			return inputStructs
			       .Accumulate(() => value.HasInputs() switch
			       {
				       true => ConstraintToStruct(value.ReturnConstraint),
				       false when value.InnerIs<Struct>(out var s) => s,
				       false when value.InnerIs<StructInstance>(out var s) => ConstraintToStruct(s.DeclaringStruct),
				       false when value.InnerIs<Instruction>(out var i) => i.LookupIntrinsicStruct(context),
				       false => context.Trace(EleMessageCode.InvalidBoundaryFunction, $"'{value}' is not recognized as a function, struct, struct instance or primitive value, cannot deduce the return type for compiling a delegate")
			       })
			       .Bind(types => types.Item1.Append(types.Item2).Select(context.ElementToClr)
			                           .BindEnumerable(clrPortTypes => Compile(value, LinqExpression.GetFuncType(clrPortTypes.ToArray()), context)));
		}

		private static Result<Delegate> Compile(IValue value, Type delegateType, Context context)
        {
	        // Check return type/single out parameter of delegate
            var method = delegateType.GetMethod(nameof(Action.Invoke));
            if (method == null) return context.Trace(EleMessageCode.InvalidBoundaryFunction, $"{delegateType} did not have invoke method");

            var delegateParameters = method.GetParameters();
            var delegateReturn = method.ReturnParameter;
            if (delegateParameters.Any(p => p.IsOut) || method.ReturnType == typeof(void))
            {
                return context.Trace(EleMessageCode.InvalidBoundaryFunction, $"{delegateType} cannot have out parameters and must have non-void return type");
            }
            
            // Create parameter expressions for input parameters and the functions return
            var parameterExpressions = delegateParameters.Select(p => LinqExpression.Parameter(p.ParameterType, p.Name)).ToArray();
            var returnExpression = LinqExpression.Parameter(delegateReturn.ParameterType, delegateReturn.Name);

            
            if (value.HasInputs() && value.InputPorts.Count != delegateParameters.Length)
            {
	            return context.Trace(EleMessageCode.InvalidBoundaryFunction, $"Function being compiled expected {value.InputPorts.Count} parameters but delegate type has {delegateParameters.Length}");
            }

            var resultBuilder = new ResultBuilder<Delegate>(context, default!);
            IValue outputExpression = value;
            // If input value is not a function we just try to use it directly
            if (value.HasInputs())
            {
	            var outputExpr = value.InputPorts
	                                  .Select((f, idx) => context.LinqToElement(parameterExpressions[idx]))
	                                  .BindEnumerable(args => value.Call(args.ToArray(), context));
	            resultBuilder.Append(in outputExpr);
	            if (outputExpr.IsError) return resultBuilder.ToResult();
	            outputExpression = outputExpr.ResultOr(default!);
            }

            var data = new CompilationData
            {
                Statements = new List<LinqExpression>(),
                Variables = new List<ParameterExpression>(),
                Cache = new Dictionary<CachedInstruction, ParameterExpression>(),
                GroupCache = new Dictionary<InstructionGroup, LinqExpression[]>()
            };

            static bool IsPrimitiveElementType(Type t) => t == typeof(float) || t == typeof(bool);
            
            // Compile delegate
            var detectCircular = new Stack<IValue>();
            Result<LinqExpression> ConvertFunction(IValue nestedValue, Type outputType, Context context)
			{
				if (detectCircular.Count >= 1 && detectCircular.Peek() == nestedValue)
				{
					return context.Trace(EleMessageCode.RecursionNotAllowed, $"Circular dependency when compiling '{nestedValue}'");
				}

				// If this value is serializable then serialize and use it
				if (nestedValue.IsSerializable(context))
				{
					return nestedValue.Serialize(context)
					         .Bind(serialized => serialized.Count switch
					         {
						         1 when IsPrimitiveElementType(outputType) => new Result<LinqExpression>(CompileInstruction(serialized[0]/*.Cache(data.CSECache, context)*/, data, context)),
						         _ => context.ElementToLinq(nestedValue, outputType, ConvertFunction)
					         });
				}

				// Else we try to use a boundary converter to convert to serializable instructions
				// TODO: Move circular checks to boundary converters
				detectCircular.Push(nestedValue);
				var retval = context.ElementToLinq(nestedValue, outputType, ConvertFunction);
				detectCircular.Pop();
				return retval;
			}

            return context.ElementToLinq(outputExpression, returnExpression.Type, ConvertFunction)
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
		
		/// <summary>
		/// Convert a CLR Object into an Element IValue.
		/// </summary>
		public static Result<IValue> ClrToElement(this IValue elementType, object clrInstance, Context context)
		{
			var serialized = new List<float>();
			return context.SerializeClrInstance(clrInstance, serialized)
			              .Bind(() => elementType.DefaultValue(context))
			              .Bind(defaultValue => defaultValue.InnerIs(out Instruction instruction)
				               // If the default value is a single instruction then this is a primitive value (Num or Bool)
				               ? serialized.Count == 1
					               ? defaultValue.Deserialize(() => new Constant(serialized[0], instruction.StructImplementation), context)
					               : context.Trace(EleMessageCode.InvalidBoundaryData, $"When converting to type {elementType} - expected {clrInstance} to serialize to 1 float but serialized to {serialized.Count} floats")
				               : elementType.GetInputPortDefaults(context)
				                            .Bind(fieldDefaults => fieldDefaults.SerializeAndFlattenValues(context))
				                            .Bind(fieldDefaultsSerialized =>
				                             {
					                             static Constant MakeConstant(float f, Instruction instruction) => new Constant(f, instruction.StructImplementation);
					                             var fieldQueue = new Queue<Instruction>(serialized.Zip(fieldDefaultsSerialized, MakeConstant));
					                             return defaultValue.Deserialize(fieldQueue.Dequeue, context);
				                             }));
		}
	}
}