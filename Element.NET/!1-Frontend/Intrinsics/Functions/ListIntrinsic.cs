using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
	public class ListIntrinsic : IntrinsicFunction
	{
		private ListIntrinsic()
		{
			Identifier = new Identifier("list");
			Inputs = new[] {Port.VariadicPort};
			Output = Port.ReturnPort(ListType.Instance);
		}
		
		public static ListIntrinsic Instance { get; } = new ListIntrinsic();

		public override Identifier Identifier { get; }
		public override IReadOnlyList<Port> Inputs { get; }
		public override Port Output { get; }
		public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
			ListType.Instance.Declaration(context.SourceContext)
			        .Map(decl => (IValue)new StructInstance(decl, new IValue[]{new IndexFunction(arguments), new Constant(arguments.Count)}));
		
		private class IndexFunction : IFunction
        {
            private readonly IReadOnlyList<IValue> _elements;

            public IndexFunction(IReadOnlyList<IValue> elements) => _elements = elements;

            IReadOnlyList<Port> IFunction.Inputs { get; } = new []{new Port("i", NumType.Instance)};
            Port IFunction.Output { get; } = Port.ReturnPort(AnyConstraint.Instance);
            IFunction IInstancable<IFunction>.GetDefinition(CompilationContext compilationContext) => this;

            public override string ToString() => "<list index function>";

            Result<IValue> IFunction.Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
                new Result<IValue>(ListElement.Create(arguments[0], _elements));
        }

        private class ListElement : IFunction, IIndexable
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

            IFunction IInstancable<IFunction>.GetDefinition(CompilationContext compilationContext) => this;

            public override string ToString() => "<list element>";
            private readonly IValue _index;
            private readonly IReadOnlyList<IValue> _elements;
            
            Result<IValue> IFunction.Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
                _elements[0] is IFunction
                    ? _elements.Select(e => ((IFunction)e).Call(arguments.ToArray(), context))
                               .MapEnumerable(v => Create(_index, v.ToList()))
                    : context.Trace(MessageCode.InvalidExpression, "List element is not a function - it cannot be called");

            public IReadOnlyList<Port> Inputs => _elements[0] is IFunction fn ? fn.Inputs : new[]{Port.VariadicPort};
            public Port Output => _elements[0] is IFunction fn ? fn.Output : Port.ReturnPort(AnyConstraint.Instance);
            public Result<IValue> this[Identifier id, bool recurse, CompilationContext context] =>
                _elements[0] is IIndexable
                    ? _elements.Select(e => ((IIndexable)e)[id, recurse, context])
                               .MapEnumerable(elements => Create(_index, elements.ToList()))
                    : context.Trace(MessageCode.InvalidExpression, "List element is not indexable");
        }
	}
}