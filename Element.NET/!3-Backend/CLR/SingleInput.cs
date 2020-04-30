namespace Element.CLR
{
	using System;
	using System.Collections.Generic;
	using LExpression = System.Linq.Expressions.Expression;

	public sealed class SingleInput : ICLRBoundaryMap
	{
		public static ICLRBoundaryMap Instance { get; } = new SingleInput();
		private SingleInput() { }

		public LExpression FromOutput(IFunction output, Type outputType, CompileFunction compile,
		                              CompilationContext context)
		{
			var result = compile(output, typeof(float), context);
			if (outputType != result.Type)
			{
				if (outputType == typeof(bool))
				{
					result = LExpression.LessThanOrEqual(result, LExpression.Constant(0f));
				}
				else
				{
					result = LExpression.Convert(result, outputType);
				}
			}

			return result;
		}

		public IFunction ToInput(LExpression parameter, ICLRBoundaryMap rootTypeMap, CompilationContext context)
		{
			var arg = parameter;
			if (arg.Type != typeof(float))
			{
				if (arg.Type == typeof(bool))
				{
					arg = LExpression.Condition(arg, LExpression.Constant(1f), LExpression.Constant(0f));
				}
				else
				{
					arg = LExpression.Convert(arg, typeof(float));
				}
			}

			return new SingleInputExpr(arg);
		}

		private class SingleInputExpr : Expression, ICLRExpression
		{
			public SingleInputExpr(LExpression parameter) => Parameter = parameter;
			public LExpression Parameter { get; }

			public override IEnumerable<Expression> Dependent => Array.Empty<Expression>();

			public LExpression Compile(Func<Expression, LExpression> compileOther) => Parameter;

			// public override bool Equals(Expression other) => this == other;

			protected override string ToStringInternal() => Parameter.ToString();
			public override bool Equals(Expression other) => (object)other == this;
			public override int GetHashCode() => Parameter.GetHashCode();
		}
	}
}