using System;
using System.Globalization;
using Element.AST;

namespace Element
{
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

		private class DummyExpression : Expression
		{
			public static DummyExpression Instance { get; } = new DummyExpression();
			public override IEnumerable<Expression> Dependent => Array.Empty<Expression>();
			protected override string ToStringInternal() => "<dummy>";
		}

		private static ReadOnlyCollection<State> ToState(IEnumerable<Expression> exprs) => exprs.Select((v, i) => new State(i, 0, v)).ToList().AsReadOnly();

		public static Result<ExpressionGroup> CreateAndOptimize(IReadOnlyCollection<Expression> initialSerialized, ConditionFunction conditionFunc, IterationFunction bodyFunc, CompilationContext context) =>
			conditionFunc(ToState(Enumerable.Repeat(DummyExpression.Instance, initialSerialized.Count)))
				.Bind(dummyConditionResult =>
				{
					// Check the condition function with non-constant dummy expressions
					if (dummyConditionResult is Constant c)
					{
						if (c == Constant.True) return context.Trace(MessageCode.InfiniteLoop, "Loop condition function always returns true");
						//if (c == Constant.False) return new BasicExpressionGroup(initialSerialized); // TODO: Implement compilation of BasicExpressionGroup and re-enable this, warn that loop is redundant
					}

					var initialState = ToState(initialSerialized);

					Result<(ReadOnlyCollection<Expression> Body, Expression Condition)> EvaluateIteration(IReadOnlyCollection<Expression> iterationState) =>
						bodyFunc(iterationState).Map(bodyExprs => new ReadOnlyCollection<Expression>(bodyExprs.ToArray()))
						                        .Accumulate(() => conditionFunc(iterationState))
						                        .Assert(e => iterationState.Count == e.Item1.Count, "Iteration state counts are different");

					Result<(ReadOnlyCollection<Expression> Body, Expression Condition)> IncrementScopeIndexIfAnyNestedLoops((ReadOnlyCollection<Expression> Body, Expression Condition) e)
					{
						var (body, condition) = e;

						// Increment the scope number if there's nested loops
						var scope = body.SelectMany(n => n.AllDependent)
						                .Concat(condition.AllDependent)
						                .OfType<State>()
						                .OrderBy(s => s.Scope)
						                .FirstOrDefault();
						if (scope != null)
						{
							initialState = initialSerialized.Select((v, i) => new State(i, scope.Scope + 1, v)).ToList().AsReadOnly();
							return EvaluateIteration(initialState);
						}

						return e;
					}

					Result<ExpressionGroup> UnrollCompileTimeConstantLoop((ReadOnlyCollection<Expression> Body, Expression Condition) e)
					{
						var (body, condition) = e;
						// TODO: Implement compilation of BasicExpressionGroup and re-enable this
						/*if (initialSerialized.All(e => e is Constant))
						{
							var builder = new ResultBuilder<ExpressionGroup>(context, null);
							var iterationCount = 1; // We already did first iteration above
							while (condition == Constant.True)
							{
								if (body.Append(condition)
								        .Any(e => !(e is Constant)))
								{
									goto NotCompileTimeConstant;
								}

								var iterationResult = EvaluateIteration(body);
								builder.Append(in iterationResult);
								if (!iterationResult.IsSuccess) return builder.ToResult();
								(body, condition) = iterationResult.ResultOr(default); // Will never pick default alternative
								
								iterationCount++;
								if (iterationCount > 100000)
								{
									return context.Trace(MessageCode.InfiniteLoop, "Iteration count exceeded 100000, likely to be an infinite loop");
								}
							}

							builder.Result = new BasicExpressionGroup(body);
							return builder.ToResult();
						}*/

						NotCompileTimeConstant:
						return new Result<ExpressionGroup>(new Loop(initialState, condition, body));
					}

					return EvaluateIteration(initialState)
					       .Bind(IncrementScopeIndexIfAnyNestedLoops)
					       .Bind(UnrollCompileTimeConstantLoop);
				});

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