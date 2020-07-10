namespace Element
{
	using System;
	using System.Collections.Generic;

	public class CachedExpression : Expression
	{
		public CachedExpression(int id, Expression value)
		{
			Id = id;
			Value = value ?? throw new ArgumentNullException(nameof(value));
		}

		public int Id { get; }
		public Expression Value { get; }

		public override IEnumerable<Expression> Dependent => new[] {Value};

		protected override string ToStringInternal() => $"C{Id}";
		public override int GetHashCode() => GetType().GetHashCode() ^ Id.GetHashCode();

		public override bool Equals(Expression other)
		{
			if (this == other) return true;
			return other is CachedExpression bOther && bOther.Id == Id;
		}
	}
}