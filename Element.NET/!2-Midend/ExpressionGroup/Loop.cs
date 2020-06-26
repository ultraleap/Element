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

		public static Result<Loop> TryCreate(IReadOnlyCollection<Expression> initialValue, ResultConditionFunction conditionFunc, ResultNewValueFunction bodyFunc)
		{
			var state = initialValue.Select((v, i) => new State(i, 0, v)).ToList().AsReadOnly();
			Result<(ReadOnlyCollection<Expression> Body, Expression Condition)> EvaluateState() =>
				bodyFunc(state).Map(bodyExprs => new ReadOnlyCollection<Expression>(bodyExprs.ToArray())).Accumulate(() => conditionFunc(state));
			
			return EvaluateState()
				.Bind(e =>
				{
					var (body, condition) = e;
					if (state.Count != body.Count) throw new InternalCompilerException("State and body expression counts are different");

					// Increment the scope number if there's nested loops
					var scope = body.SelectMany(n => n.AllDependent)
					                .Concat(condition.AllDependent)
					                .OfType<State>()
					                .OrderBy(s => s.Scope)
					                .FirstOrDefault();
					if (scope != null)
					{
						state = initialValue.Select((v, i) => new State(i, scope.Id + 1, v)).ToList().AsReadOnly();
						return EvaluateState().Map(e1 => new Loop(state, e1.Condition, e1.Body));
					}

					return new Loop(state, condition, body);
				});
		}

		public static Loop Create(IEnumerable<Expression> initialValue, ConditionFunction conditionFunc, NewValueFunction bodyFunc)
		{
			var initialExpressions = initialValue as Expression[] ?? initialValue.ToArray();
			var state = initialExpressions.Select((v, i) => new State(i, 0, v)).ToList().AsReadOnly();
			(ReadOnlyCollection<Expression> Body, Expression Condition) EvaluateState() => (new ReadOnlyCollection<Expression>(bodyFunc(state).ToArray()), conditionFunc(state));
			
			var (body, condition) = EvaluateState();
			if (state.Count != body.Count) throw new InternalCompilerException("State and body expression counts are different");

			// Increment the scope number if there's nested loops
			var scope = body.SelectMany(n => n.AllDependent)
			                .Concat(condition.AllDependent)
			                .OfType<State>()
			                .OrderBy(s => s.Scope)
			                .FirstOrDefault();
			if (scope != null)
			{
				state = initialExpressions.Select((v, i) => new State(i, scope.Id + 1, v)).ToList().AsReadOnly();
				(body, condition) = EvaluateState();
			}

			return new Loop(state, condition, body);
		}
		
		private Loop(ReadOnlyCollection<State> state, Expression condition, ReadOnlyCollection<Expression> body)
		{
			State = state;
			Condition = condition;
			Body = body;
		}

		public override IEnumerable<Expression> Dependent => State.Concat(Body).Concat(new[] {Condition});

		protected override string ToStringInternal() => $"Loop({StateListJoin(State)}; {Condition}; {ListJoin(Body)})";
		// public override bool Equals(Expression other) => this == other || other is Loop && other.ToString() == ToString();
	}
}