using Element.AST;

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

		public static void Optimize(Expression[] inputs, Mode mode, Dictionary<Expression, CachedExpression> cache)
		{
			for (var i = 0; i < inputs.Length; i++)
			{
				inputs[i] = OptimizeSingle(cache, inputs[i]);
			}
			if (mode == Mode.OnlyMultipleUses)
			{
				// TODO: Not supported yet
			}
		}

		public static Expression OptimizeSingle(Dictionary<Expression, CachedExpression> cache, Expression value)
		{
			if (value is Constant || value is CachedExpression || value is State) { return value; }
			if (!cache.TryGetValue(value, out var found))
			{
				Expression newValue;
				switch (value)
				{
					case Unary u:
						newValue = new Unary(u.Operation, OptimizeSingle(cache, u.Operand));
						break;
					case Binary b:
						newValue = new Binary(b.Operation, OptimizeSingle(cache, b.OpA), OptimizeSingle(cache, b.OpB));
						break;
					case Mux m:
						newValue = new Mux(OptimizeSingle(cache, m.Selector), m.Operands.Select(o => OptimizeSingle(cache, o)));
						break;
					case ExpressionGroupElement ge:
						return new ExpressionGroupElement(OptimizeGroup(cache, ge.Group), ge.Index);
					default:
						return value;
				}
				cache.Add(value, found = new CachedExpression(cache.Count, newValue));
			}
			return found;
		}

		private static ExpressionGroup OptimizeGroup(Dictionary<Expression, CachedExpression> cache, ExpressionGroup group)
		{
			// TODO: Generalise this somehow?
			switch (group)
			{
				case Persist p:
					return new Persist(p.State.Select(s => OptimizeSingle(cache, s.InitialValue)),
						_ => p.NewValue.Select(n => OptimizeSingle(cache, n)));
				case Loop l:
					return new Loop(l.State.Select(s => OptimizeSingle(cache, s.InitialValue)),
						_ => OptimizeSingle(cache, l.Condition),
						_ => l.Body.Select(n => OptimizeSingle(cache, n)));
				default:
					return group;
			}
		}

		private static Expression FoldBackSingleUses(CachedExpression[] singleUses, Expression value)
		{
			switch (value)
			{
				case CachedExpression c:
					if (System.Array.IndexOf(singleUses, c) > 0) {
						return c.Value;
					}
					break;
				case Unary u:
					return new Unary(u.Operation, FoldBackSingleUses(singleUses, u.Operand));
				case Binary b:
					return new Binary(b.Operation, FoldBackSingleUses(singleUses, b.OpA), FoldBackSingleUses(singleUses, b.OpB));
			}
			return value;
		}
	}
}