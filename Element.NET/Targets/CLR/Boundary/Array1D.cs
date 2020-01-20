namespace Element.CLR
{
	using System;
	using System.Collections.Generic;

	// TODO: Change this to be a generic array translator
	// TODO: Support List, other collection types as well
	public sealed class Array1D : ICLRBoundaryMap
	{
		public System.Linq.Expressions.Expression FromOutput(IFunction output, Type outputType, CompileFunction compile,
		                                                     CompilationContext context)
		{
			throw new NotImplementedException("Outputting an array is not supported yet");
		}

		public IFunction ToInput(System.Linq.Expressions.Expression parameter, ICLRBoundaryMap rootTypeMap, CompilationContext info) =>
			new Array1DFunc(parameter);

		private class Array1DFunc : IFunction
		{
			public Array1DFunc(System.Linq.Expressions.Expression parameter) => _parameter = parameter;
			private readonly System.Linq.Expressions.Expression _parameter;
			public PortInfo[] Inputs => Array.Empty<PortInfo>();

			public PortInfo[] Outputs { get; } = {
				new PortInfo{Name = "count", Type = NumberType.Instance},
				new PortInfo{Name = "index", Type = AnyType.Instance}
			};

			public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
			{
				if (this.CheckArguments(arguments, output, context) != null)
				{
					return CompilationError.Instance;
				}

				if (output == "count")
				{
					return SingleInput.Instance.ToInput(System.Linq.Expressions.Expression.ArrayLength(_parameter), null, context);
				}

				return new Array1DIndexer(_parameter);
			}
		}

		private class Array1DIndexer : IFunction
		{
			public Array1DIndexer(System.Linq.Expressions.Expression parameter) => _parameter = parameter;
			private readonly System.Linq.Expressions.Expression _parameter;

			public PortInfo[] Inputs { get; } =
			{
				new PortInfo {Name = "i", Type = NumberType.Instance}
			};

			public PortInfo[] Outputs { get; } = {
				new PortInfo{Name = "return", Type = NumberType.Instance}
			};

			public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
			{
				return this.CheckArguments(arguments, output, context)
					?? (IFunction)new ArrayIndex(_parameter, (Expression)arguments[0]);
			}
		}

		private class ArrayIndex : Expression, ICLRExpression
		{
			public ArrayIndex(System.Linq.Expressions.Expression array, Expression index)
			{
				Array = array;
				Index = index;
			}

			public System.Linq.Expressions.Expression Array { get; }
			public Expression Index { get; }
			public override IEnumerable<Expression> Dependent => new[] {Index};

			public System.Linq.Expressions.Expression Compile(Func<Expression, System.Linq.Expressions.Expression> compileOther) =>
				System.Linq.Expressions.Expression.ArrayIndex(Array, compileOther(Index));

			protected override string ToStringInternal() => $"{Array}[{Index}]";
		}
	}
}