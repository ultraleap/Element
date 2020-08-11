using System;

namespace Element
{
	using System.Collections.Generic;
	using System.Collections.ObjectModel;
	using System.Linq;

	/// <summary>
	/// A group representing repeated iteration until a condition is not satisfied. Each iteration is a group item.
	/// </summary>
	public class Loop : InstructionGroup
	{
		public override int Size => State.Count;
		public ReadOnlyCollection<State> State { get; }
		public Instruction Condition { get; }
		public ReadOnlyCollection<Instruction> Body { get; }

		private class DummyInstruction : Instruction
		{
			public static DummyInstruction Instance { get; } = new DummyInstruction();
			public override IEnumerable<Instruction> Dependent => Array.Empty<Instruction>();
			public override string SummaryString => "<dummy>";
		}

		private static ReadOnlyCollection<State> ToState(IEnumerable<Instruction> exprs) => exprs.Select((v, i) => new State(i, 0, v)).ToList().AsReadOnly();

		public static Result<InstructionGroup> CreateAndOptimize(IReadOnlyCollection<Instruction> initialSerialized, ConditionFunction conditionFunc, IterationFunction bodyFunc, Context context) =>
			conditionFunc(ToState(Enumerable.Repeat(DummyInstruction.Instance, initialSerialized.Count)))
				.Bind(dummyConditionResult =>
				{
					// Check the condition function with non-constant dummy instructions
					if (dummyConditionResult is Constant c)
					{
						// ReSharper disable once PossibleUnintendedReferenceComparison
						if (c == Constant.True) return context.Trace(MessageCode.InfiniteLoop, "Loop condition function always returns true");
						//if (c == Constant.False) return new BasicInstructionGroup(initialSerialized); // TODO: Implement compilation of BasicInstructionGroup and re-enable this, warn that loop is redundant
					}

					var initialState = ToState(initialSerialized);

					Result<(ReadOnlyCollection<Instruction> Body, Instruction Condition)> EvaluateIteration(IReadOnlyCollection<Instruction> iterationState) =>
						bodyFunc(iterationState!).Map(bodyExprs => new ReadOnlyCollection<Instruction>(bodyExprs.ToArray()))
						                        .Accumulate(() => conditionFunc(iterationState!))
						                        .Assert(e => iterationState!.Count == e.Item1.Count, "Iteration state counts are different");

					Result<(ReadOnlyCollection<Instruction> Body, Instruction Condition)> IncrementScopeIndexIfAnyNestedLoops((ReadOnlyCollection<Instruction> Body, Instruction Condition) e)
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

					Result<InstructionGroup> UnrollCompileTimeConstantLoop((ReadOnlyCollection<Instruction> Body, Instruction Condition) e)
					{
						var (body, condition) = e;
						// TODO: Implement compilation of BasicInstructionGroup and re-enable this
						/*if (initialSerialized.All(e => e is Constant))
						{
							var builder = new ResultBuilder<InstructionGroup>(context, null);
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

							builder.Result = new BasicInstructionGroup(body);
							return builder.ToResult();
						}

						NotCompileTimeConstant:*/
						return new Result<InstructionGroup>(new Loop(initialState, condition, body));
					}

					return EvaluateIteration(initialState)
					       .Bind(IncrementScopeIndexIfAnyNestedLoops)
					       .Bind(UnrollCompileTimeConstantLoop);
				});

		private Loop(ReadOnlyCollection<State> state, Instruction condition, ReadOnlyCollection<Instruction> body)
		{
			State = state;
			Condition = condition;
			Body = body;
		}

		public override IEnumerable<Instruction> Dependent => State.Concat(Body).Concat(new[] {Condition});

		public override string SummaryString => $"Loop({StateListJoin(State)}; {Condition}; {ListJoinToString(Body)})";
		// public override bool Equals(Instruction other) => this == other || other is Loop && other.ToString() == ToString();
	}
}