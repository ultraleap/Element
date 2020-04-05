namespace Element
{
	using System;
	using System.Collections.Generic;

	/// <summary>
	/// Single arity expression, i.e. f(a) = expr
	/// </summary>
	public class Unary : Expression
	{
		public enum Op
		{
			Sin,
			Cos,
			Tan,
			ASin,
			ACos,
			ATan,
			Ln,
			Abs,
			Ceil,
			Floor
		}

		public static float Evaluate(Op op, float a) =>
			op switch
			{
				Op.Sin => (float) Math.Sin(a),
				Op.Cos => (float) Math.Cos(a),
				Op.Tan => (float) Math.Tan(a),
				Op.ASin => (float) Math.Asin(a),
				Op.ACos => (float) Math.Acos(a),
				Op.ATan => (float) Math.Atan(a),
				Op.Ln => (float) Math.Log(a),
				Op.Abs => Math.Abs(a),
				Op.Ceil => (float) Math.Ceiling(a),
				Op.Floor => (float) Math.Floor(a),
				_ => throw new ArgumentOutOfRangeException()
			};

		public Unary(Op operation, Expression operand)
		{
			Operation = operation;
			Operand = operand ?? throw new ArgumentNullException(nameof(operand));
		}

		public Op Operation { get; }
		public Expression Operand { get; }

		public override IEnumerable<Expression> Dependent => new[] {Operand};
		protected override string ToStringInternal() => $"{Operation}({Operand})";
		public override int GetHashCode() => (int)Operation ^ Operand.GetHashCode();
		public override bool Equals(Expression other) =>
			this == other || other is Unary bOther
			&& bOther.Operation == Operation
			&& bOther.Operand.Equals(Operand);
	}
}