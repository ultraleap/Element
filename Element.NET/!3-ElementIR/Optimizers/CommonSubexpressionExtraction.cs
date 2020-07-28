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

		public static void CacheExpressions(this Expression[] inputs, Mode mode, Dictionary<Expression, CachedExpression> cache)
		{
			for (var i = 0; i < inputs.Length; i++)
			{
				inputs[i] = CacheExpressions(inputs[i], cache);
			}
			if (mode == Mode.OnlyMultipleUses)
			{
				// TODO: Not supported yet
			}
		}

		public static Expression CacheExpressions(this Expression value, Dictionary<Expression, CachedExpression> cache)
		{
			if (value is Constant || value is CachedExpression || value is State) { return value; }
			if (!cache.TryGetValue(value, out var found))
			{
				Expression newValue;
				switch (value)
				{
					case Unary u:
						newValue = new Unary(u.Operation, CacheExpressions(u.Operand, cache));
						break;
					case Binary b:
						newValue = new Binary(b.Operation, CacheExpressions(b.OpA, cache), CacheExpressions(b.OpB, cache));
						break;
					case Mux m:
						newValue = new Mux(CacheExpressions(m.Selector, cache), m.Operands.Select(o => CacheExpressions(o, cache)));
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

		private static ExpressionGroup OptimizeGroup(Dictionary<Expression, CachedExpression> cache, ExpressionGroup group) =>
			group switch
			{
				/*Loop l => Loop.CreateAndOptimize(l.State.Select(s => CacheExpressions(s.InitialValue, cache)).ToArray(),
				                                 _ => CacheExpressions(l.Condition, cache),
				                                 _ => new Result<IEnumerable<Expression>>(l.Body.Select(n => CacheExpressions(n, cache))),
				                                 )
				              .Match((expression, messages) => (Loop)expression, // TODO: Do something with any potential warnings
				                     messages => throw new InternalCompilerException("Subexpression extraction should not cause errors")),*/
				_ => group
			};

		private static Expression FoldBackSingleUses(CachedExpression[] singleUses, Expression value) =>
			value switch
			{
				CachedExpression c when System.Array.IndexOf(singleUses, c) > 0 => c.Value,
				Unary u => new Unary(u.Operation, FoldBackSingleUses(singleUses, u.Operand)),
				Binary b => new Binary(b.Operation, FoldBackSingleUses(singleUses, b.OpA), FoldBackSingleUses(singleUses, b.OpB)),
				_ => value
			};
	}
}