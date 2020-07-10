namespace Element
{
	using System;
	using System.Collections.Generic;
	using System.Collections.ObjectModel;
	using System.Linq;

	/// <summary>
	/// Multiplexing expression, picks a result from many arguments based on passed in selector function
	/// </summary>
	public class Mux : Expression
	{
		public Mux(Expression selector, IEnumerable<Expression> operands)
		{
			Selector = selector ?? throw new ArgumentNullException(nameof(selector));
			Operands = new ReadOnlyCollection<Expression>(operands.ToArray());
			if (Operands.Any(o => o == null))
			{
				throw new ArgumentException("An operand was null");
			}
		}

		public Expression Selector { get; }
		public ReadOnlyCollection<Expression> Operands { get; }

		public override IEnumerable<Expression> Dependent => Operands.Concat(new[] {Selector});
		protected override string ToStringInternal() => $"[{ListJoin(Operands)}]:{Selector}";

		private int? _hashCode;

		public override int GetHashCode()
		{
			if (!_hashCode.HasValue)
			{
				var c = Selector.GetHashCode();
				for (var i = 0; i < Operands.Count; i++)
				{
					c ^= Operands[i].GetHashCode();
				}

				_hashCode = c;
			}

			return _hashCode.Value;
		}

		public override bool Equals(Expression other)
		{
			if (this == other) return true;
			if (!(other is Mux bOther) || bOther.Operands.Count != Operands.Count || !bOther.Selector.Equals(Selector))
				return false;
			for (var i = 0; i < Operands.Count; i++)
			{
				if (!Operands[i].Equals(bOther.Operands[i]))
				{
					return false;
				}
			}

			return true;
		}
	}
}