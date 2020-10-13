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
		public bool IsVariadic => true;

		public override Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) =>
			context.RootScope.Lookup(ListStruct.Instance.Identifier, context)
			       .Accumulate(() => context.RootScope.Lookup(NumStruct.Instance.Identifier, context))
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

            protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, Context context) =>
	            arguments[0] is Instruction index
		            ? ListElement.Create(index,
		                                 _elements,
		                                 _elements[0].IsFunction ? _elements[0].InputPorts : new[] {ResolvedPort.VariadicPort},
		                                 _elements[0].IsFunction ? _elements[0].ReturnConstraint : AnyConstraint.Instance,
		                                 context)
		            : context.Trace(EleMessageCode.ConstraintNotSatisfied, "List Index must be a Num");
        }

        private class ListElement : Function
        {
	        public static Result<IValue> Create(Instruction index, IReadOnlyList<IValue> elements, IReadOnlyList<ResolvedPort> inputConstraints, IValue outputConstraint, Context context) =>
		        index.CompileTimeIndex(0, elements.Count, context).Bind(index => new Result<IValue>(elements[index]))
		             .Else(() => elements.All(e => e is Instruction)
			                         ? Switch.CreateAndOptimize(index, elements.Cast<Instruction>(), context).Cast<IValue>(context)
			                         : new Result<IValue>(new ListElement(index, elements, inputConstraints, outputConstraint)));

            private ListElement(Instruction index, IReadOnlyList<IValue> elements, IReadOnlyList<ResolvedPort> inputPorts, IValue output)
            {
                _index = index;
                _elements = elements;
                InputPorts = inputPorts;
                ReturnConstraint = output;
            }

            private readonly Instruction _index;
            private readonly IReadOnlyList<IValue> _elements;
            public override string SummaryString => $"ListElement({_index})[{string.Join(", ", _elements)}]";
            public override IReadOnlyList<ResolvedPort> InputPorts { get; }
            public override IValue ReturnConstraint { get; }

            public override Result<IValue> Index(Identifier id, Context context) =>
	            _elements.Select(e => e.Index(id, context))
	                     .BindEnumerable(elements => Create(_index, elements.ToList(), InputPorts, ReturnConstraint, context));
            
            protected override Result<IValue> ResolveCall(IReadOnlyList<IValue> arguments, Context context) =>
	            _elements.Select(e => e.Call(arguments.ToArray(), context))
	                     .BindEnumerable(v => Create(_index, v.ToList(), InputPorts, ReturnConstraint, context));
        }
	}
}