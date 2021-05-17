using Element.AST;
using ResultNET;

namespace Element
{
	using System;
	using System.Collections.Generic;
	using System.Collections.ObjectModel;
	using System.Linq;

	/// <summary>
	/// Switch instruction, picks a result from many arguments based on passed in selector function
	/// </summary>
	public class Switch : Instruction
	{
		public static Result<Instruction> CreateAndOptimize(Instruction selector, IEnumerable<Instruction> operands, Context context)
		{
			var options = operands as Instruction[] ?? operands.ToArray();
			if (options.Length < 1) return context.Trace(EleMessageCode.ArgumentCountMismatch, "Switch requires at least 1 option");
			if (options.Length == 1 || options.All(o => o.Equals(options[0]))) return options[0];

			return options[0].LookupIntrinsicStruct(context)
			                 .Bind(expectedType =>
			                 {
				                 Result<Instruction> HasCompileTimeIndex(int idx) => new Result<Instruction>(options[idx]);
				                 Result<Instruction> NotCompileTimeIndexable() => new Switch(selector, options, options[0].StructImplementation);

				                 return options.Cast<IValue>()
				                               .ToList()
				                               .VerifyValuesAreAllOfInstanceType(expectedType, () => selector.CompileTimeIndex(0, options.Length, context)
				                                                                                             .Branch(HasCompileTimeIndex, NotCompileTimeIndexable),
				                                                                 context);
			                 });
		}
		
		private Switch(Instruction selector, IEnumerable<Instruction> operands, IIntrinsicStructImplementation type) : base(type)
		{
			Selector = selector ?? throw new ArgumentNullException(nameof(selector));
			Operands = new ReadOnlyCollection<Instruction>(operands.ToArray());
			if (Operands.Any(o => o == null))
			{
				throw new ArgumentException("An operand was null");
			}
		}

		public Instruction Selector { get; }
		public ReadOnlyCollection<Instruction> Operands { get; }

		public override IEnumerable<Instruction> Dependent => Operands.Concat(new[] {Selector});
		public override string SummaryString => $"Switch(${Selector})[{ListJoinToString(Operands)}]";

		private int? _hashCode;

		public override int GetHashCode()
		{
			if (!_hashCode.HasValue)
			{
				var c = Selector.GetHashCode();
				for (var i = 0; i < Operands.Count; i++)
				{
					c ^= Operands[i].GetHashCode();
				}

				_hashCode = c;
			}

			return _hashCode.Value;
		}

		public override bool Equals(Instruction other)
		{
			if (this == other) return true;
			if (!(other is Switch bOther) || bOther.Operands.Count != Operands.Count || !bOther.Selector.Equals(Selector))
				return false;
			for (var i = 0; i < Operands.Count; i++)
			{
				if (!Operands[i].Equals(bOther.Operands[i]))
				{
					return false;
				}
			}

			return true;
		}
	}
}