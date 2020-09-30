using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;

namespace Element
{
	/// <summary>
	/// Base class for all Element instructions
	/// </summary>
	// TODO: Make a wrapper Value class for instructions to remove Value logic from Instruction implementation
	public abstract class Instruction : Value, IEquatable<Instruction>
	{
		protected Instruction(IIntrinsicStructImplementation? instanceStructImplementationOverride = default) => StructImplementation = instanceStructImplementationOverride ?? NumStruct.Instance;
		
		/// <summary>
		/// The primitive type of this value
		/// </summary>
		public readonly IIntrinsicStructImplementation StructImplementation;

		public abstract IEnumerable<Instruction> Dependent { get; }

		protected static string ListJoinToString(IEnumerable<Instruction> list) => string.Join(", ", list.Select(e => e.ToString()));
		protected static string ListJoinNormalizedForm(IEnumerable<Instruction> list) => string.Join(", ", list.Select(e => e.NormalizedFormString));
		
		public abstract override string SummaryString { get; }
		public override string TypeOf => StructImplementation.Identifier.String;
		public override string NormalizedFormString => ListJoinNormalizedForm(Dependent);

		public override bool Equals(object obj) => obj is Instruction instruction && Equals(instruction);
		public virtual bool Equals(Instruction other) => other?.ToString() == ToString();
		public override int GetHashCode() => ToString().GetHashCode();
		
		public IEnumerable<Instruction> AllDependent => Dependent.SelectMany(d => new[] {d}.Concat(d.AllDependent));
		public int CountUses(Instruction other) => Equals(other) ? 1 : Dependent.Sum(d => d.CountUses(other));

		// TODO: This allows indexing constants from an instruction, see note about wrapper class above to fix
		public override Result<IValue> Index(Identifier id, Context context) =>
			context.RootScope.Lookup(StructImplementation.Identifier, context).Cast<Struct>(context).Bind(v => v.ResolveInstanceFunction(this, id, context));

		public override void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context) => resultBuilder.Result.Add(this);

		public override Result<IValue> Deserialize(Func<Instruction> nextValue, Context context)
		{
			var result = nextValue();
			return result.StructImplementation == StructImplementation
				       ? new Result<IValue>(result)
				       : context.Trace(MessageCode.SerializationError, $"'{result}' deserialized to incorrect type: is '{result.StructImplementation}' - expected '{StructImplementation}'");
		}
	}
}