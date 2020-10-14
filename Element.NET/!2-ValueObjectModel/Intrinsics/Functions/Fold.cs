using System.Collections.Generic;

namespace Element.AST
{
	public sealed class Fold : IntrinsicValue, IIntrinsicFunctionImplementation
	{
		private Fold()
		{
			Identifier = new Identifier("fold");
		}
		
		public static Fold Instance { get; } = new Fold();
		public override Identifier Identifier { get; }
		public bool IsVariadic => false;
		public override Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context)
		{
			var list = (StructInstance) arguments[0];
			var workingValue = arguments[1];
			var aggregator = arguments[2];
			
			// TODO: Simplify dynamic fold to a loop expression
			// Source implementation for a dynamic fold
			// _(list:List, initial, aggregator:Binary) = for({i = 0; value = initial;};,
			//			_(tup):Bool = tup.i.lt(count),
			//			_(tup) = {i = tup.i.add(1); value = aggregator(tup.value, list.at(tup.i); };)

			static SourceInfo ConditionSourceCode() => new SourceInfo("<compiler generated dynamic fold condition>",
			                                                         "_(tup):Bool = tup.i.lt(list.count)");
			static SourceInfo BodySourceCode() => new SourceInfo("<compiler generated dynamic fold body>",
			                                                     "_(tup) = {i = tup.i.add(1), value = aggregator(tup.value, list.at(tup.i)) }");
			Result<IValue[]> CreateDynamicFoldArguments() =>
				Parser.Parse<Lambda>(ConditionSourceCode(), context)
				      .Accumulate(() => Parser.Parse<Lambda>(BodySourceCode(), context))
				      .Bind(t =>
				      {
					      var (predicateLambda, bodyLambda) = t;
					      var initial = new ResolvedBlock(new (Identifier, IValue)[]
					      {
						      (new Identifier("i"), Constant.Zero),
						      (new Identifier("value"), workingValue)
					      }, null);
					      var captureBlock = new ResolvedBlock(new (Identifier, IValue)[]
					      {
						      (new Identifier("list"), list),
						      (new Identifier("initial"), initial),
						      (new Identifier("aggregator"), aggregator)
					      }, context.RootScope);
					      return predicateLambda.ResolveExpression(captureBlock, context)
					                            .Accumulate(() => bodyLambda.ResolveExpression(captureBlock, context))
					                            .Map(r =>
					                            {
						                            var (predicateFunction, bodyFunction) = r;
						                            return new[] {initial, predicateFunction, bodyFunction};
					                            });
				      });

			return ListStruct.EvaluateElements(list, context)
			                 .FoldArray(workingValue, (current, e) => aggregator.Call(new[] {current, e}, context))
			                 .Else(() => CreateDynamicFoldArguments()
			                             .Bind(args => For.Instance.Call(args, context))
			                             .Bind(instance => instance.IndexPositionally(1, context)));
		}
	}
}