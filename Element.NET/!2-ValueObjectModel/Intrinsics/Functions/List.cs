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
	            arguments[0].IsType(out Instruction index)
		            ? HomogenousListElement.Create(index, _elements, context)
		            : context.Trace(EleMessageCode.ConstraintNotSatisfied, "List Index must be a Num");
        }

        public class HomogenousListElement : WrapperValue
        {
	        public static Result<IValue> Create(Instruction index, IReadOnlyList<IValue> elements, Context context) =>
		        index.CompileTimeIndex(0, elements.Count, context)
		             .Branch(compileConstantIndex => compileConstantIndex < elements.Count
			                                           ? new Result<IValue>(elements[compileConstantIndex])
			                                           : context.Trace(EleMessageCode.ArgumentOutOfRange, $"Index {compileConstantIndex} out of range - list has {elements.Count} items"),
		                     () =>
		                     {
			                     var operands = new Instruction[elements.Count];

			                     for (var i = 0; i < elements.Count; i++)
			                     {
				                     // If any elements are not instructions then we need to 
				                     if (elements[i].IsType(out Instruction instruction)) operands[i] = instruction;
				                     else return new Result<IValue>(new HomogenousListElement(index, elements));
			                     }

			                     return Switch.CreateAndOptimize(index, operands, context).Cast<IValue>();
		                     });

            private HomogenousListElement(Instruction index, IReadOnlyList<IValue> elements)
				: base(elements[0])
            {
                _index = index;
                _elements = elements;
            }

            private readonly Instruction _index;
            private readonly IReadOnlyList<IValue> _elements;
            public override string SummaryString => $"HomogenousListElement({_index})[{string.Join(", ", _elements)}]";
            public override bool IsFunction => _elements[0].IsFunction;

            public override Result<IValue> Index(Identifier id, Context context) =>
	            _elements.Select(e => e.Index(id, context))
	                     .BindEnumerable(elements => Create(_index, elements.ToList(), context));

            public override Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) =>
	            _elements.Select(e => e.Call(arguments.ToArray(), context))
	                     .BindEnumerable(v => Create(_index, v.ToList(), context));
        }
	}
}