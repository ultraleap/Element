using System;

namespace Element.AST
{
	public class ListIntrinsic : IntrinsicFunction
	{
		public ListIntrinsic()
			: base("list",
			       new[] {Port.VariadicPort},
			       Port.ReturnPort(ListType.Instance))
		{ }
		
		public override IValue Call(IValue[] arguments, CompilationContext context) =>
			ListType.Instance.ResolveCall(new IValue[]{new IndexFunction(arguments), new Constant(arguments.Length)}, null, false, context);

		private class IndexFunction : IFunction
		{
			private readonly IValue[] _elements;

			public IndexFunction(IValue[] elements) => _elements = elements;

			IType IValue.Type => FunctionType.Instance;
			Port[] IFunctionSignature.Inputs { get; } = {new Port("i", NumType.Instance)};
			Port IFunctionSignature.Output { get; } = Port.ReturnPort(AnyConstraint.Instance);
			IFunctionSignature IFunctionSignature.GetDefinition(CompilationContext compilationContext) => this;

			public override string ToString() => "<list index function>";

			IValue IFunction.Call(IValue[] arguments, CompilationContext context) =>
				// TODO: check argument are homogenous
				_elements[(int)Math.Round(arguments[0] as Constant, MidpointRounding.AwayFromZero)];
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