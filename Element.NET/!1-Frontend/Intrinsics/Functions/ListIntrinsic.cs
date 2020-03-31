using System;

namespace Element.AST
{
	public class ListIntrinsic : IIntrinsic, IFunction
	{
		public IType Type => FunctionType.Instance;
		public string Location => "list";
		public Port[] Inputs { get; } = {Port.VariadicPort};
		public Port Output { get; } = Port.ReturnPort(ListType.Instance);
		public IValue Call(IValue[] arguments, CompilationContext context) =>
			ListType.Instance.Call(new IValue[]{new IndexFunction(arguments), new Literal(arguments.Length), }, context);

		private class IndexFunction : ICallable
		{
			public IType Type => FunctionType.Instance;
			public IndexFunction(IValue[] elements) => _elements = elements;

			public override string ToString() => "<list index function>";

			private readonly IValue[] _elements;
			public IValue Call(IValue[] arguments, CompilationContext context)
			{
				// TODO check argument are homogenous
				var validArgCount =  arguments.ValidateArguments(1, context);
				if (!validArgCount) return CompilationErr.Instance;
				if (!(arguments[0] is Literal indexLiteral)) return context.LogError(8, "Argument must be a Num");
				var index = (int)Math.Round(indexLiteral, MidpointRounding.AwayFromZero);
				return _elements[index];
			}
		}

		/*private class Indexer : ICallable
		{
			/*public static IFunction Create(IFunction index, IFunction[] elements, CompilationContext info)
			{
				if (index.AsExpression(info) != null
					&& elements.All(e => e.AsExpression(info) != null))
				{
					return new Mux(index.AsExpression(info), elements.Select(e => e.AsExpression(info)));
				}

				if (elements.Any(e => e.IsLeaf()))
				{
					return NumberType.Instance;
				}

				return new Indexer(index, elements);
			}#1#

			public Indexer(IValue[] elements) => _elements = elements;

			public override string ToString() => "<list element>";
			private readonly IValue[] _elements;

			public IValue Call(IValue[] arguments, CompilationContext context)
			{
				// TODO type checking
				var validArgCount =  arguments.ValidateArguments(1, context);
				if (!validArgCount) return null;
				if (!(arguments[0] is Literal indexLiteral)) return null;
				var index = (int)Math.Round(indexLiteral, MidpointRounding.AwayFromZero);
				return _elements[index];
			}

			public IType Type => FunctionType.Instance;
		}*/
	}
}