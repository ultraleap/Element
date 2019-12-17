namespace Element
{
	using System;
	using System.Collections.Generic;

	/// <summary>
	/// An expression which represents a constant value
	/// </summary>
	public class Constant : Expression
	{
		public static Constant Zero { get; } = new Constant(0);
		public static Constant One { get; } = new Constant(1);

		public Constant(float value) => Value = value;

		public override IEnumerable<Expression> Dependent => Array.Empty<Expression>();
		public float Value { get; }

		protected override string ToStringInternal() => Value.ToString();
		public override int GetHashCode() => Value.GetHashCode();
		public override bool Equals(Expression other) => (other as Constant)?.Value == Value;
	}
}