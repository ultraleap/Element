using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
	public sealed class FoldIntrinsic : IntrinsicFunction
	{
		public FoldIntrinsic()
			: base("List.fold",
			       new[]
			       {
				       new Port("list", ListType.Instance),
				       new Port("initial", AnyConstraint.Instance),
				       new Port("aggregator", BinaryFunctionConstraint.Instance)
			       }, Port.ReturnPort(AnyConstraint.Instance))
		{ }

		private class FoldCaptureScope : ScopeBase, IDeclared
		{
			private readonly IScope _parent;

			public FoldCaptureScope(Declaration declarer, IEnumerable<(Identifier, IValue)> items, IScope parent)
			{
				Declarer = declarer;
				_parent = parent;
				SetRange(items);
			}

			public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
				IndexCache(id) ?? (recurse ? _parent[id, true, compilationContext] : null);

			public Declaration Declarer { get; }
		}
		
		public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
		{
			var list = arguments[0] as StructInstance;
			var workingValue = arguments[1];
			var aggregator = (IFunctionSignature)arguments[2];
			

			// TODO: Simplify dynamic fold to a loop expression
			// Source implementation for a dynamic fold
			// _(list:List, initial, aggregator:Binary) = for(Tuple(0, initial),
			//			_(tup):Bool = tup.varg0.lt(count),
			//			_(tup) = Tuple(tup.varg0.add(1), aggregator(tup.varg1, list.at(tup.varg0)))
			IValue[] CreateDynamicFoldArguments()
			{
				var initial = TupleType.Instance.ResolveCall(new []{Constant.Zero, workingValue}, false, compilationContext);
				if (!compilationContext.Parse("_(tup):Bool = tup.varg0.lt(list.count)", out Lambda predicateLambda)) throw new InternalCompilerException("Couldn't parse dynamic fold condition");
				if (!compilationContext.Parse("_(tup) = Tuple(tup.varg0.add(1), aggregator(tup.varg1, list.at(tup.varg0)))", out Lambda bodyLambda)) throw new InternalCompilerException("Couldn't parse dynamic fold body");
				var declaration = compilationContext.GetIntrinsicsDeclaration<Declaration>(this);
				predicateLambda.Initialize(declaration);
				bodyLambda.Initialize(declaration);
				var captureScope = new FoldCaptureScope(declaration, new (Identifier, IValue)[]
				{
					(new Identifier("list"), list),
					(new Identifier("aggregator"), aggregator)
				}, declaration.Parent);
				var predicateFn = predicateLambda.ResolveExpression(captureScope, compilationContext);
				var bodyFn = bodyLambda.ResolveExpression(captureScope, compilationContext);

				return new[] {initial, predicateFn, bodyFn};
			}
			
			return ListType.GetListCount(list, compilationContext) switch
			{
				(ListType.CountType.Constant, int count) => ListType.EvaluateElements(list, ListType.CountType.Constant, count, compilationContext)
				                                                    .Aggregate(workingValue, (current, e) => aggregator.ResolveCall(new[] {current, e}, false, compilationContext)),
				(ListType.CountType.Dynamic, _) => ((StructInstance) IntrinsicCache.GetByLocation<IFunctionSignature>("for", compilationContext)
				                                                                   .ResolveCall(CreateDynamicFoldArguments(), false, compilationContext))[1],
				_ => CompilationError.Instance
			};
		}
	}
}