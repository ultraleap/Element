using Element.AST;

namespace Element
{
	using System;
	using System.Collections.Generic;

	/// <summary>
	/// 2-arity expression, i.e. f(a, b) = expr
	/// </summary>
	public class Binary : Expression
	{
		public enum Op
		{
			// Bool
			And,
			Or,
			Eq,
			NEq,
			Lt,
			LEq,
			Gt,
			GEq,
			
			// Num
			Add,
			Sub,
			Mul,
			Div,
			Rem,
			Pow,
			Min,
			Max,
			Log,
			Atan2,
		}

		public static Constant Evaluate(Op op, float a, float b) =>
			op switch
			{
				Op.And => a * b > 0f ? Constant.True : Constant.False,
				Op.Or => (a + b) - (a * b) > 0f ? Constant.True : Constant.False,
				Op.Eq => Unary.Evaluate(Unary.Op.Not, Evaluate(Op.NEq, a, b).Value),
				Op.NEq => Math.Abs(a - b) > 0f ? Constant.True : Constant.False,
				Op.Lt => b - a > 0f ? Constant.True : Constant.False,
				Op.Gt => a - b > 0f ? Constant.True : Constant.False,
				Op.LEq => Unary.Evaluate(Unary.Op.Not, Evaluate(Op.Gt, a, b).Value),
				Op.GEq => Unary.Evaluate(Unary.Op.Not, Evaluate(Op.Lt, a, b).Value),
				_ => new Constant(op switch
				{
					Op.Add => (a + b),
					Op.Sub => (a - b),
					Op.Mul => (a * b),
					Op.Div => (a / b),
					Op.Rem => (a % b),
					Op.Pow => (float) Math.Pow(a, b),
					Op.Min => Math.Min(a, b),
					Op.Max => Math.Max(a, b),
					Op.Log => (float) Math.Log(a, b),
					Op.Atan2 => (float) Math.Atan2(a, b),
					_ => throw new ArgumentOutOfRangeException(nameof(op), op, null)
				})
			};

		public Binary(Op operation, Expression opA, Expression opB)
			: base(operation switch
			{
				Op.And => BoolStructImplementation.Instance,
				Op.Or => BoolStructImplementation.Instance,
				Op.Eq => BoolStructImplementation.Instance,
				Op.NEq => BoolStructImplementation.Instance,
				Op.Lt => BoolStructImplementation.Instance,
				Op.LEq => BoolStructImplementation.Instance,
				Op.Gt => BoolStructImplementation.Instance,
				Op.GEq => BoolStructImplementation.Instance,
				_ => default
			})
		{
			Operation = operation;
			OpA = opA ?? throw new ArgumentNullException(nameof(opA));
			OpB = opB ?? throw new ArgumentNullException(nameof(opB));
		}

		public Op Operation { get; }
		public Expression OpA { get; }
		public Expression OpB { get; }

		public override IEnumerable<Expression> Dependent => new[] {OpA, OpB};

		protected override string ToStringInternal() => $"{Operation}({OpA}, {OpB})";
		public override int GetHashCode() => (int)Operation ^ OpA.GetHashCode() ^ OpB.GetHashCode();

		public override bool Equals(Expression other) =>
			this == other || other is Binary bOther
			&& bOther.Operation == Operation
			&& bOther.OpA.Equals(OpA)
			&& bOther.OpB.Equals(OpB);
	}
}