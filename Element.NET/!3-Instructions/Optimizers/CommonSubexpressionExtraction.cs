namespace Element
{
	using System.Linq;
	using System.Collections.Generic;

	public static class CommonSubexpressionExtraction
	{
		public enum Mode
		{
			EveryOperation,
			OnlyMultipleUses
		}

		public static void Cache(this Instruction[] inputs, Mode mode, Dictionary<Instruction, CachedInstruction> cache)
		{
			for (var i = 0; i < inputs.Length; i++)
			{
				inputs[i] = Cache(inputs[i], cache);
			}
			if (mode == Mode.OnlyMultipleUses)
			{
				// TODO: Not supported yet
			}
		}

		public static Instruction Cache(this Instruction value, Dictionary<Instruction, CachedInstruction> cache)
		{
			if (value is Constant || value is CachedInstruction || value is State) { return value; }
			if (!cache.TryGetValue(value, out var found))
			{
				Instruction newValue;
				switch (value)
				{
					case Unary u:
						newValue = Unary.CreateAndOptimize(u.Operation, Cache(u.Operand, cache));
						break;
					case Binary b:
						newValue = Binary.CreateAndOptimize(b.Operation, Cache(b.OpA, cache), Cache(b.OpB, cache));
						break;
					case Mux m:
						newValue = Mux.CreateAndOptimize(Cache(m.Selector, cache), m.Operands.Select(o => Cache(o, cache)));
						break;
					case InstructionGroupElement ge:
						return new InstructionGroupElement(OptimizeGroup(cache, ge.Group), ge.Index);
					default:
						return value;
				}
				cache.Add(value, found = new CachedInstruction(cache.Count, newValue));
			}
			return found;
		}

		private static InstructionGroup OptimizeGroup(Dictionary<Instruction, CachedInstruction> cache, InstructionGroup group) =>
			group switch
			{
				/*Loop l => Loop.CreateAndOptimize(l.State.Select(s => Cache(s.InitialValue, cache)).ToArray(),
				                                 _ => Cache(l.Condition, cache),
				                                 _ => new Result<IEnumerable<Instruction>>(l.Body.Select(n => Cache(n, cache))),
				                                 )
				              .Match((expression, messages) => (Loop)expression, // TODO: Do something with any potential warnings
				                     messages => throw new InternalCompilerException("Subexpression extraction should not cause errors")),*/
				_ => group
			};

		private static Instruction FoldBackSingleUses(CachedInstruction[] singleUses, Instruction value) =>
			value switch
			{
				CachedInstruction c when System.Array.IndexOf(singleUses, c) > 0 => c.Value,
				Unary u => Unary.CreateAndOptimize(u.Operation, FoldBackSingleUses(singleUses, u.Operand)),
				Binary b => Binary.CreateAndOptimize(b.Operation, FoldBackSingleUses(singleUses, b.OpA), FoldBackSingleUses(singleUses, b.OpB)),
				_ => value
			};
	}
}