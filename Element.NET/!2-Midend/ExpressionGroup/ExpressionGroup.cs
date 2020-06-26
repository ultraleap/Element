using Element.AST;

namespace Element
{
	using System;
	using System.Collections.Generic;
	using System.Collections.ObjectModel;

	/// <summary>
	/// Base class for expressions which include multiple grouped expressions requiring evaluation
	/// </summary>
	public abstract class ExpressionGroup : Expression
	{
		public abstract int Size { get; }
	}

	public delegate Expression ConditionFunction(ReadOnlyCollection<State> state);
	public delegate Result<Expression> ResultConditionFunction(ReadOnlyCollection<State> state);

	public delegate IEnumerable<Expression> NewValueFunction(ReadOnlyCollection<State> previous);
	public delegate Result<IEnumerable<Expression>> ResultNewValueFunction(ReadOnlyCollection<State> previous);

	/// <summary>
	/// A single expression within an expression group
	/// </summary>
	public class ExpressionGroupElement : Expression
	{
		public ExpressionGroupElement(ExpressionGroup group, int index)
		{
			if (group == null) { throw new ArgumentNullException(nameof(group)); }

			if (index < 0 || index >= group.Size) { throw new ArgumentOutOfRangeException(nameof(index)); }

			Group = group;
			Index = index;
		}

		public ExpressionGroup Group { get; }
		public int Index { get; }

		public override IEnumerable<Expression> Dependent => Group.Dependent;
		protected override string ToStringInternal() => $"{Group}.{Index}";
		public override int GetHashCode() => GetType().GetHashCode() ^ Group.GetHashCode() ^ Index;

		public override bool Equals(Expression other)
		{
			if (this == other) return true;
			return other is ExpressionGroupElement bOther && bOther.Index == Index && bOther.Group.Equals(Group);
		}
	}
}