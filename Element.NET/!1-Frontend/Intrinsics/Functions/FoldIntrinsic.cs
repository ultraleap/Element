using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
	public sealed class FoldIntrinsic : IntrinsicFunctionSignature
	{
		private FoldIntrinsic()
		{
			Identifier = new Identifier("fold");
		}
		
		public static FoldIntrinsic Instance { get; } = new FoldIntrinsic();

		private class FoldCaptureScope : Scope
		{
			public FoldCaptureScope(IEnumerable<(Identifier, IValue)> items, IScope parent)
			{
				Parent = parent;
				_source = items.ToList();
			}

			public override IScope? Parent { get; }
			protected override IList<(Identifier Identifier, IValue Value)> _source { get; }
		}

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

			static string ConditionSourceCode() => "_(tup):Bool = tup.varg0.lt(list.count)";
			static string BodySourceCode() => "_(tup) = Tuple(tup.varg0.add(1), aggregator(tup.varg1, list.at(tup.varg0)))";
			Result<IValue[]> CreateDynamicFoldArguments() =>
				context.SourceContext.GlobalScope.Lookup(TupleType.Instance.Identifier, context)
				       .Cast<StructDeclaration>(context)
				       .Bind(tupleDecl => TupleType.Instance.Construct(tupleDecl, new[] {Constant.Zero, workingValue}, context)
				                                   .Accumulate(() => Parser.Parse<Lambda>(ConditionSourceCode(), context),
				                                               () => Parser.Parse<Lambda>(BodySourceCode(), context))
				                                   .Bind(t =>
				                                   {
					                                   var (initial, predicateLambda, bodyLambda) = t;
					                                   predicateLambda.Initialize(tupleDecl);
					                                   bodyLambda.Initialize(tupleDecl);
					                                   var captureScope = new FoldCaptureScope(new (Identifier, IValue)[]
					                                   {
						                                   (new Identifier("list"), list),
						                                   (new Identifier("aggregator"), aggregator)
					                                   }, tupleDecl.Parent);
					                                   return predicateLambda.ResolveExpression(captureScope, context)
					                                                         .Accumulate(() => bodyLambda.ResolveExpression(captureScope, context))
					                                                         .Map(r =>
					                                                         {
						                                                         var (predicateFunction, bodyFunction) = r;
						                                                         return new[] {initial, predicateFunction, bodyFunction};
					                                                         });
				                                   }));

			return ListType.EvaluateElements(list, context)
			               .FoldArray(workingValue, (current, e) => aggregator.Call(new[] {current, e}, context))
			               .Else(() => CreateDynamicFoldArguments()
			                           .Bind(args => ForIntrinsic.Instance.Call(args, context))
			                           .Cast<StructInstance>(context)
			                           .Map(instance => instance.Members[1]));
		}
	}
}