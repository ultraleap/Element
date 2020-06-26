using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
	public sealed class FoldIntrinsic : IntrinsicFunction
	{
		private FoldIntrinsic()
		{
			Identifier = new Identifier("fold");
			Inputs = new[]
			{
				new Port("list", ListType.Instance),
				new Port("initial", AnyConstraint.Instance),
				new Port("aggregator", BinaryFunctionConstraint.Instance)
			};
			Output = Port.ReturnPort(AnyConstraint.Instance);
		}
		
		public static FoldIntrinsic Instance { get; } = new FoldIntrinsic();

		private class FoldCaptureScope : ScopeBase, IDeclared
		{
			private readonly IScope _parent;

			public FoldCaptureScope(Declaration declarer, IEnumerable<(Identifier, IValue)> items, IScope parent)
			{
				Declarer = declarer;
				_parent = parent;
				_source = items.ToList();
			}

			public override Result<IValue> this[Identifier id, bool recurse, CompilationContext context] =>
				Index(id, context).ElseIf(recurse, () =>  _parent[id, true, context]);

			protected override IList<(Identifier Identifier, IValue Value)> _source { get; }

			public Declaration Declarer { get; }
			public int IndexInSource => Declarer.IndexInSource;
		}

		public override Identifier Identifier { get; }
		public override IReadOnlyList<Port> Inputs { get; }
		public override Port Output { get; }
		public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context)
		{
			var list = (StructInstance) arguments[0];
			var workingValue = arguments[1];
			var aggregator = (IFunction)arguments[2];
			
			// TODO: Simplify dynamic fold to a loop expression
			// Source implementation for a dynamic fold
			// _(list:List, initial, aggregator:Binary) = for(Tuple(0, initial),
			//			_(tup):Bool = tup.varg0.lt(count),
			//			_(tup) = Tuple(tup.varg0.add(1), aggregator(tup.varg1, list.at(tup.varg0)))

			static string ConditionSourceCode() => "_(tup):Bool = tup.varg0.lt(list.count)";
			static string BodySourceCode() => "_(tup) = Tuple(tup.varg0.add(1), aggregator(tup.varg1, list.at(tup.varg0)))";
			Result<IValue[]> CreateDynamicFoldArguments() =>
				TupleType.Instance.Construct(new[] {Constant.Zero, workingValue}, context)
				         .Accumulate(() => Parser.Parse<Lambda>(ConditionSourceCode(), context),
				                     () => Parser.Parse<Lambda>(BodySourceCode(), context),
				                     () => Declaration(context.SourceContext))
				         .Bind(tuple =>
				         {
					         var (initial, predicateLambda, bodyLambda, declaration) = tuple;
					         predicateLambda.Initialize(declaration, null);
					         bodyLambda.Initialize(declaration, null);
					         var captureScope = new FoldCaptureScope(declaration, new (Identifier, IValue)[]
					         {
						         (new Identifier("list"), list),
						         (new Identifier("aggregator"), aggregator)
					         }, declaration.Parent);
					         return predicateLambda.ResolveExpression(captureScope, context)
					                               .Accumulate(() => bodyLambda.ResolveExpression(captureScope, context))
					                               .Map(r =>
					                               {
						                               var (predicateFunction, bodyFunction) = r;
						                               return new[] {initial, predicateFunction, bodyFunction};
					                               });
				         });

			return ListType.EvaluateElements(list, context)
			               .FoldArray(workingValue, (current, e)  => aggregator.Call(new[] {current, e}, context))
			               .Else(() => CreateDynamicFoldArguments()
				                     .Bind(args => ForIntrinsic.Instance.Call(args, context))
				                     .Cast<StructInstance>(context)
				                     .Bind(instance => instance.Field(1, context)));
		}
	}
}