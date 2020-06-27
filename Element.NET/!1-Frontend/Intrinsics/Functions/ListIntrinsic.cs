using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
	public class ListIntrinsic : IntrinsicFunctionSignature
	{
		private ListIntrinsic()
		{
			Identifier = new Identifier("list");
		}
		
		public static ListIntrinsic Instance { get; } = new ListIntrinsic();

		public override Identifier Identifier { get; }

		public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
			context.SourceContext.GlobalScope.Lookup(ListType.Instance.Identifier, context)
			       .Bind(decl => decl.Call(new IValue[] {new IndexFunctionSignature(arguments), new Constant(arguments.Count)}, context));
		
		private class IndexFunctionSignature : Value, IFunctionSignature
        {
            private readonly IReadOnlyList<IValue> _elements;

            public IndexFunctionSignature(IReadOnlyList<IValue> elements) => _elements = elements;

            IReadOnlyList<Port> IFunctionSignature.Inputs { get; } = new []{new Port("i", NumType.Instance)};
            Port IFunctionSignature.Output { get; } = Port.ReturnPort(AnyConstraint.Instance);

            public override string ToString() => "<list index function>";

            public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
                new Result<IValue>(ListElement.Create(arguments[0], _elements));
        }

        private class ListElement : Value, IFunctionSignature
        {
            public static IValue Create(IValue index, IReadOnlyList<IValue> elements) =>
                index switch
                {
                    Element.Expression indexExpr when elements.All(e => e is Element.Expression) => new Mux(indexExpr, elements.Cast<Element.Expression>()),
                    Constant constantIndex => elements[(int)constantIndex.Value],
                    _ => new ListElement(index, elements)
                };

            private ListElement(IValue index, IReadOnlyList<IValue> elements)
            {
                _index = index;
                _elements = elements;
            }


            public override string ToString() => "<list element>";
            private readonly IValue _index;
            private readonly IReadOnlyList<IValue> _elements;

            public IReadOnlyList<Port> Inputs => _elements[0] is IFunctionSignature fn ? fn.Inputs : new[]{Port.VariadicPort};
            public Port Output => _elements[0] is IFunctionSignature fn ? fn.Output : Port.ReturnPort(AnyConstraint.Instance);
            public override Result<IValue> Index(Identifier id, CompilationContext context) =>
	            _elements.Select(e => e.Index(id, context))
	                     .MapEnumerable(elements => Create(_index, elements.ToList()));
            public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
	            _elements.Select(e => e.Call(arguments.ToArray(), context))
	                     .MapEnumerable(v => Create(_index, v.ToList()));
        }
	}
}