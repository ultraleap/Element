using System.Linq;

namespace Element
{
	using System;
	using System.Collections.Generic;

	/// <summary>
	/// Base class for expressions which include multiple grouped expressions requiring evaluation
	/// </summary>
	public abstract class ExpressionGroup : Expression
	{
		public abstract int Size { get; }
		public override string TypeOf => "ExpressionGroup";
		public static string StateListJoin(IEnumerable<State> list) => string.Join(", ", list.Select(e => e.InitialValue.ToString()));
	}

	public class BasicExpressionGroup : ExpressionGroup
	{
		private readonly IReadOnlyCollection<Expression> _expressions;

		public BasicExpressionGroup(IReadOnlyCollection<Expression> expressions) => _expressions = expressions;

		public override IEnumerable<Expression> Dependent => _expressions;
		public override string SummaryString => $"Group({ListJoinToString(_expressions)})";

		public override int Size => _expressions.Count;
	}

	public delegate Result<Expression> ConditionFunction(IReadOnlyCollection<Expression> state);
	public delegate Result<IEnumerable<Expression>> IterationFunction(IReadOnlyCollection<Expression> previous);

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
		public override string SummaryString => $"{Group}.{Index}";
		public override int GetHashCode() => GetType().GetHashCode() ^ Group.GetHashCode() ^ Index;

		public override bool Equals(Expression other)
		{
			if (this == other) return true;
			return other is ExpressionGroupElement bOther && bOther.Index == Index && bOther.Group.Equals(Group);
		}
	}
}