using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
	public class List : IntrinsicValue, IIntrinsicFunctionImplementation
	{
		private List()
		{
			Identifier = new Identifier("list");
		}
		
		public static List Instance { get; } = new List();
		public override Identifier Identifier { get; }

		public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
			context.SourceContext.GlobalScope.Lookup(ListStruct.Instance.Identifier, context)
			       .Cast<IntrinsicStruct>(context)
			       .Accumulate(() => context.SourceContext.GlobalScope.Lookup(NumStruct.Instance.Identifier, context))
			       .Bind(t =>
			       {
				       var (listStruct, numStruct) = t;
				       return listStruct.Call(new IValue[]
				       {
					       new ListIndexer(arguments, new[]{new ResolvedPort(numStruct)}, AnyConstraint.Instance),
					       new Constant(arguments.Count)
				       }, context);
			       });
		
		private class ListIndexer : Function
        {
            private readonly IReadOnlyList<IValue> _elements;

            public ListIndexer(IReadOnlyList<IValue> elements, IReadOnlyList<ResolvedPort> inputPorts, IValue output)
            {
	            _elements = elements;
	            InputPorts = inputPorts;
	            ReturnConstraint = output;
            }

            public override IReadOnlyList<ResolvedPort> InputPorts { get; }
            public override IValue ReturnConstraint { get; }

            protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, CompilationContext context) =>
	            arguments[0] is Element.Expression indexIr
		                               ? new Result<IValue>(ListElement.Create(indexIr,
		                                                                       _elements,
		                                                                       _elements[0] is IFunctionSignature f ? f.InputPorts : new[] {ResolvedPort.VariadicPort},
		                                                                       _elements[0] is IFunctionSignature fn ? fn.ReturnConstraint : AnyConstraint.Instance))
		                               : context.Trace(MessageCode.ConstraintNotSatisfied, "List indexer must be an Element IR value");
        }

        private class ListElement : Function
        {
            public static IValue Create(Element.Expression index, IReadOnlyList<IValue> elements, IReadOnlyList<ResolvedPort> inputConstraints, IValue outputConstraint) =>
                index switch
                {
	                Constant constantIndex => elements[(int) constantIndex.Value],
	                {} indexExpr => elements.All(e => e is Element.Expression)
		                                ? (IValue) new Mux(indexExpr, elements.Cast<Element.Expression>())
		                                : new ListElement(index, elements, inputConstraints, outputConstraint),
	                _ => throw new ArgumentNullException(nameof(index))
                };

            private ListElement(Element.Expression index, IReadOnlyList<IValue> elements, IReadOnlyList<ResolvedPort> inputPorts, IValue output)
            {
                _index = index;
                _elements = elements;
                InputPorts = inputPorts;
                ReturnConstraint = output;
            }

            private readonly Element.Expression _index;
            private readonly IReadOnlyList<IValue> _elements;
            public override IReadOnlyList<ResolvedPort> InputPorts { get; }
            public override IValue ReturnConstraint { get; }

            public override Result<IValue> Index(Identifier id, CompilationContext context) =>
	            _elements.Select(e => e.Index(id, context))
	                     .MapEnumerable(elements => Create(_index, elements.ToList(), InputPorts, ReturnConstraint));
            
            protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, CompilationContext context) =>
	            _elements.Select(e => e.Call(arguments.ToArray(), context))
	                     .MapEnumerable(v => Create(_index, v.ToList(), InputPorts, ReturnConstraint));
        }
	}
}