namespace Element
{
	using System;
	using System.Collections.Generic;
	using System.Collections.ObjectModel;
	using System.Linq;

	/// <summary>
	/// A looped repeating expression. Each loop iteration is an expression group item.
	/// </summary>
	public class Loop : ExpressionGroup
	{
		public override int Size => State.Count;
		public ReadOnlyCollection<State> State { get; }
		public Expression Condition { get; }
		public ReadOnlyCollection<Expression> Body { get; }

		public Loop(IEnumerable<Expression> initialValue, ConditionFunction condition, NewValueFunction body)
		{
			if (condition == null) throw new ArgumentNullException(nameof(condition));
			State = initialValue.Select((v, i) => new State(i, 0, v)).ToList().AsReadOnly();
			Body = new ReadOnlyCollection<Expression>(body(State).ToArray());
			Condition = condition(State);
			if (Body.Any(e => e == null))
			{
				throw new ArgumentException("An operand was null");
			}

			if (State.Count != Body.Count)
			{
				throw new ArgumentException("Collection counts were different");
			}

			// Increment the scope number if there's nested loops
			var scope = Body.SelectMany(n => n.AllDependent)
			                .Concat(Condition.AllDependent)
			                .OfType<State>()
			                .OrderBy(s => s.Scope)
			                .FirstOrDefault();
			if (scope != null)
			{
				State = initialValue.Select((v, i) => new State(i, scope.Id + 1, v)).ToList().AsReadOnly();
				Body = new ReadOnlyCollection<Expression>(body(State).ToArray());
				Condition = condition(State);
			}
		}

		public override IEnumerable<Expression> Dependent => State.Concat(Body).Concat(new[] {Condition});

		protected override string ToStringInternal() => $"Loop({StateListJoin(State)}; {Condition}; {ListJoin(Body)})";
		// public override bool Equals(Expression other) => this == other || other is Loop && other.ToString() == ToString();
	}
}