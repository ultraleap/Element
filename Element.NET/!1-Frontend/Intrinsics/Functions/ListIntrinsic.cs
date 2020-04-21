using System;
using System.Linq;

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
			// TODO: check argument are homogenous
			ListType.Instance.ResolveCall(new IValue[]{new IndexFunction(arguments), new Constant(arguments.Length)}, false, context);

		private class IndexFunction : IFunction
		{
			private readonly IValue[] _elements;

			public IndexFunction(IValue[] elements) => _elements = elements;

			IType IValue.Type => FunctionType.Instance;
			Port[] IFunctionSignature.Inputs { get; } = {new Port("i", NumType.Instance)};
			Port IFunctionSignature.Output { get; } = Port.ReturnPort(AnyConstraint.Instance);
			IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;

			public override string ToString() => "<list index function>";

			IValue IFunction.Call(IValue[] arguments, CompilationContext context) =>
				Indexer.Create(arguments[0], _elements);
		}

		private class Indexer : IFunction
		{
			public static IValue Create(IValue index, IValue[] elements) =>
				index is Element.Expression indexExpr && elements.All(e => e is Element.Expression)
					? (IValue) new Mux(indexExpr, elements.Cast<Element.Expression>())
					: new Indexer(index, elements);

			private Indexer(IValue index, IValue[] elements)
			{
				_index = index;
				_elements = elements;
			}

			IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;

			public override string ToString() => "<list element>";
			private readonly IValue _index;
			private readonly IValue[] _elements;

			IValue IFunction.Call(IValue[] arguments, CompilationContext context) => Create(_index, _elements);

			public IType Type => FunctionType.Instance;
			public Port[] Inputs { get; } = Array.Empty<Port>();
			public Port Output { get; } = Port.ReturnPort(AnyConstraint.Instance);
		}
	}
}