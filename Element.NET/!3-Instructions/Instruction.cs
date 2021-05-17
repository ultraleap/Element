using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;
using ResultNET;

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

		private Result<Struct>? _intrinsicStruct;
		public Result<Struct> LookupIntrinsicStruct(Context context) => _intrinsicStruct ??= context.RootScope
		                                                                                            .Lookup(StructImplementation.Identifier, context)
		                                                                                            .Bind(result => result.InnerIs<Struct>(out var @struct)
			                                                                                                            ? new Result<Struct>(@struct)
			                                                                                                            : context.Trace(EleMessageCode.IntrinsicNotFound, $"'{result}' is not intrinsic struct declaration for {StructImplementation.Identifier}"));
		public virtual Result<Constant> CompileTimeConstant(Context context) => context.Trace(EleMessageCode.NotCompileConstant, $"'{this}' is not a constant");

		public abstract IEnumerable<Instruction> Dependent { get; }

		protected static string ListJoinToString(IEnumerable<Instruction> list) => string.Join(", ", list.Select(e => e.ToString()));
		
		public abstract override string SummaryString { get; }
		public override string TypeOf => StructImplementation.Identifier.String;
		public override Result<IValue> InstanceType(Context context) => LookupIntrinsicStruct(context).Cast<IValue>();

		public override bool Equals(object obj) => obj is Instruction instruction && Equals(instruction);
		public virtual bool Equals(Instruction other) => other?.ToString() == ToString();
		public override int GetHashCode() => ToString().GetHashCode();
		
		public IEnumerable<Instruction> AllDependent => Dependent.SelectMany(d => new[] {d}.Concat(d.AllDependent));
		public int CountUses(Instruction other) => Equals(other) ? 1 : Dependent.Sum(d => d.CountUses(other));

		// TODO: This allows indexing constants from an instruction, see note about wrapper class above to fix
		public override Result<IValue> Index(Identifier id, Context context) =>
			LookupIntrinsicStruct(context).Bind(v => v.ResolveInstanceFunction(this, id, context));

		public override void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context) => resultBuilder.Result.Add(this);

		public override Result<IValue> Deserialize(Func<Instruction> nextValue, Context context)
		{
			var result = nextValue();
			return result.StructImplementation == StructImplementation
				       ? new Result<IValue>(result)
				       : context.Trace(EleMessageCode.SerializationError,
				                       $"'{result}' deserialized to incorrect type: is '{result.StructImplementation}' - expected '{StructImplementation}'");
		}
	}

	public static class InstructionExtensions
	{
		public static Result<int> ConstantToIndex(this Constant constant, int min, int max, Context context)
		{
			if (float.IsNaN(constant.Value))
			{
				// TODO: Return error type, not an error message
				return context.Trace(EleMessageCode.ArgumentOutOfRange, "Constant was NaN - cannot convert to an index");
			}

			var asInt = (int) constant.Value;
			var inRange = asInt >= min && asInt <= max;
			return inRange
				       ? new Result<int>(asInt)
				       // TODO: Return error type, not an error message
				       : context.Trace(EleMessageCode.ArgumentOutOfRange, $"Index '{asInt}' not in range of [{min}, {max}]");
		}

		public static Result<int> CompileTimeIndex(this Instruction instruction, int min, int max, Context context) =>
			instruction.CompileTimeConstant(context).Bind(constant => constant.ConstantToIndex(min, max, context));
	}
}