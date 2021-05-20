using System.Linq;
using ResultNET;

namespace Element
{
	using System;
	using System.Collections.Generic;

	/// <summary>
	/// Base class for groups of 
	/// </summary>
	public abstract class InstructionGroup : Instruction
	{
		public abstract int Size { get; }
		public override string TypeOf => "InstructionGroup";
		public static string StateListJoin(IEnumerable<State> list) => string.Join(", ", list.Select(e => e.InitialValue.ToString()));
	}

	public class BasicInstructionGroup : InstructionGroup
	{
		private readonly IReadOnlyCollection<Instruction> _instructions;

		public BasicInstructionGroup(IReadOnlyCollection<Instruction> instructions) => _instructions = instructions;

		public override IEnumerable<Instruction> Dependent => _instructions;
		public override string SummaryString => $"Group({ListJoinToString(_instructions)})";

		public override int Size => _instructions.Count;
	}

	public delegate Result<Instruction> ConditionFunction(IReadOnlyCollection<Instruction> state);
	public delegate Result<IEnumerable<Instruction>> IterationFunction(IReadOnlyCollection<Instruction> previous);

	/// <summary>
	/// A single item in a group.
	/// </summary>
	public class InstructionGroupElement : Instruction
	{
		public InstructionGroupElement(InstructionGroup group, int groupIndex)
		{
			if (group == null) { throw new ArgumentNullException(nameof(group)); }

			if (groupIndex < 0 || groupIndex >= group.Size) { throw new ArgumentOutOfRangeException(nameof(groupIndex)); }

			Group = group;
			GroupIndex = groupIndex;
		}

		public InstructionGroup Group { get; }
		public int GroupIndex { get; }

		public override IEnumerable<Instruction> Dependent => Group.Dependent;
		public override string SummaryString => $"{Group}.{GroupIndex}";
		public override int GetHashCode() => GetType().GetHashCode() ^ Group.GetHashCode() ^ GroupIndex;

		public override bool Equals(Instruction other)
		{
			// ReSharper disable once PossibleUnintendedReferenceComparison
			if (this == other) return true;
			return other is InstructionGroupElement bOther && bOther.GroupIndex == GroupIndex && bOther.Group.Equals(Group);
		}
	}
}