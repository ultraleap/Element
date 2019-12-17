namespace Element
{
	using System;
	using System.Collections.Generic;

	public class State : Expression
	{
		public override IEnumerable<Expression> Dependent => new[] {InitialValue};
		public int Id { get; }
		public int Scope { get; }
		public Expression InitialValue { get; }

		public State(int id, int scope, Expression initialValue)
		{
			Id = id;
			Scope = scope;
			InitialValue = initialValue ?? throw new ArgumentNullException(nameof(initialValue));
		}

		protected override string ToStringInternal() => $"$State<{Id}>";
		public override int GetHashCode() => GetType().GetHashCode() ^ Id.GetHashCode() ^ Scope.GetHashCode();

		public override bool Equals(Expression other)
		{
			if (this == other) return true;
			return other is State bOther && bOther.Id == Id && bOther.Scope == Scope;
		}
	}
}