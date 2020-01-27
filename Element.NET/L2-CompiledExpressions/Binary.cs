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

		public static float Evaluate(Op op, float a, float b) =>
			op switch
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
				_ => throw new ArgumentOutOfRangeException()
			};

		public Binary(Op operation, Expression opA, Expression opB)
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

		public override bool Equals(Expression other)
		{
			if (this == other) return true;
			return other is Binary bOther
				&& bOther.Operation == Operation
				&& bOther.OpA.Equals(OpA)
				&& bOther.OpB.Equals(OpB);
		}
	}
}