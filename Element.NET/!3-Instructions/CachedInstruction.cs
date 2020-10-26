namespace Element
{
	using System;
	using System.Collections.Generic;

	public class CachedInstruction : Instruction
	{
		public CachedInstruction(int id, Instruction value)
		{
			Id = id;
			Value = value ?? throw new ArgumentNullException(nameof(value));
		}

		public int Id { get; }
		public Instruction Value { get; }

		public override IEnumerable<Instruction> Dependent => new[] {Value};

		public override string SummaryString => $"C{Id}";
		public override int GetHashCode() => GetType().GetHashCode() ^ Id.GetHashCode();

		public override bool Equals(Instruction other)
		{
			// ReSharper disable once PossibleUnintendedReferenceComparison
			if (this == other) return true;
			return other is CachedInstruction bOther && bOther.Id == Id;
		}
	}
}