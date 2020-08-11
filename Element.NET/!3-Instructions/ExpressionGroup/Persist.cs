/*namespace Element
{
	using System;
	using System.Collections.Generic;
	using System.Collections.ObjectModel;
	using System.Linq;

	public class Persist : InstructionGroup
	{
		public override int Size => State.Count;
		public ReadOnlyCollection<State> State { get; }
		public ReadOnlyCollection<Instruction> NewValue { get; }

		public Persist(IEnumerable<Instruction> initialValue, NewValueFunction newValue)
		{
			State = initialValue.Select((v, i) => new State(i, 0, v)).ToList().AsReadOnly();
			NewValue = new ReadOnlyCollection<Instruction>(newValue(State).ToArray());
			if (NewValue.Any(e => e == null))
			{
				throw new ArgumentException("An operand was null");
			}

			if (State.Count != NewValue.Count)
			{
				throw new ArgumentException("Collection counts were different");
			}

			// Increment the scope number if there's nested loops
			var scope = NewValue.SelectMany(n => n.AllDependent).OfType<State>().OrderBy(s => s.Scope).FirstOrDefault();
			if (scope != null)
			{
				State = initialValue.Select((v, i) => new State(i, scope.Id + 1, v)).ToList().AsReadOnly();
				NewValue = new ReadOnlyCollection<Instruction>(newValue(State).ToArray());
			}
		}

		public override IEnumerable<Instruction> Dependent => State.Concat(NewValue);

		protected override string ToStringInternal() => $"Persist({StateListJoin(State)}; {ListJoinToString(NewValue)})";
		// public override bool Equals(Instruction other) => this == other || other is Persist && other.ToString() == ToString();
	}
}*/