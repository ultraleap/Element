using System.Collections.Generic;

namespace Element.AST
{
	public sealed class FoldIntrinsicFunctionImplementation : IntrinsicFunctionImplementation
	{
		private FoldIntrinsicFunctionImplementation()
		{
			Identifier = new Identifier("fold");
		}
		
		public static FoldIntrinsicFunctionImplementation Instance { get; } = new FoldIntrinsicFunctionImplementation();

		public override Identifier Identifier { get; }
		public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context)
		{
			var list = (StructInstance) arguments[0];
			var workingValue = arguments[1];
			var aggregator = arguments[2];
			
			// TODO: Simplify dynamic fold to a loop expression
			// Source implementation for a dynamic fold
			// _(list:List, initial, aggregator:Binary) = for(Tuple(0, initial),
			//			_(tup):Bool = tup.varg0.lt(count),
			//			_(tup) = Tuple(tup.varg0.add(1), aggregator(tup.varg1, list.at(tup.varg0)))

			static SourceInfo ConditionSourceCode() => new SourceInfo("<compiler generated dynamic fold condition>", "_(tup):Bool = tup.varg0.lt(list.count)");
			static SourceInfo BodySourceCode() => new SourceInfo("<compiler generated dynamic fold body", "_(tup) = Tuple(tup.varg0.add(1), aggregator(tup.varg1, list.at(tup.varg0)))");
			Result<IValue[]> CreateDynamicFoldArguments() =>
				context.SourceContext.GlobalScope.Lookup(TupleStructImplementation.Instance.Identifier, context)
				       .Cast<Struct>(context)
				       .Bind(tupleDecl => TupleStructImplementation.Instance.Construct(tupleDecl, new[] {Constant.Zero, workingValue}, context)
				                                   .Accumulate(() => Parser.Parse<Lambda>(ConditionSourceCode(), context),
				                                               () => Parser.Parse<Lambda>(BodySourceCode(), context))
				                                   .Bind(t =>
				                                   {
					                                   var (initial, predicateLambda, bodyLambda) = t;
					                                   var captureScope = new Scope(new (Identifier, IValue)[]
					                                   {
						                                   (new Identifier("list"), list),
						                                   (new Identifier("aggregator"), aggregator)
					                                   }, null);
					                                   return predicateLambda.ResolveExpression(captureScope, context)
					                                                         .Accumulate(() => bodyLambda.ResolveExpression(captureScope, context))
					                                                         .Map(r =>
					                                                         {
						                                                         var (predicateFunction, bodyFunction) = r;
						                                                         return new[] {initial, predicateFunction, bodyFunction};
					                                                         });
				                                   }));

			return ListStructImplementation.EvaluateElements(list, context)
			               .FoldArray(workingValue, (current, e) => aggregator.Call(new[] {current, e}, context))
			               .Else(() => CreateDynamicFoldArguments()
			                           .Bind(args => ForIntrinsicFunctionImplementation.Instance.Call(args, context))
			                           .Cast<StructInstance>(context)
			                           .Bind(instance => instance.IndexPositionally(1, context)));
		}
	}
}