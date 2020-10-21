namespace Element
{
	using System;
	using System.Collections.Generic;

	public class State : Instruction
	{
		public override IEnumerable<Instruction> Dependent => new[] {InitialValue};
		public int Id { get; }
		public int Scope { get; }
		public Instruction InitialValue { get; }

		public State(int id, int scope, Instruction initialValue) :base(initialValue.StructImplementation)
		{
			Id = id;
			Scope = scope;
			InitialValue = initialValue ?? throw new ArgumentNullException(nameof(initialValue));
		}

		public override string SummaryString => $"$State<{Id}:{InitialValue.SummaryString}>";
		public override int GetHashCode() => GetType().GetHashCode() ^ Id.GetHashCode() ^ Scope.GetHashCode();

		public override bool Equals(Instruction other)
		{
			if (this == other) return true;
			return other is State bOther && bOther.Id == Id && bOther.Scope == Scope;
		}
	}
}