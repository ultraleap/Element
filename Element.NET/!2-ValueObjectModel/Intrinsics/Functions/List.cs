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
	            arguments[0].InnerIs(out Instruction index)
		            ? HomogenousListElement.Create(index, _elements, context)
		            : context.Trace(EleMessageCode.ConstraintNotSatisfied, "List Index must be a Num");
        }

		private class HomogenousListElement : IValue
		{
			public static Result<IValue> Create(Instruction index, IReadOnlyList<IValue> elements, Context context) =>
				elements[0].InstanceType(context)
				           .Bind(listElementInstanceType =>
					                 elements.VerifyValuesAreAllOfInstanceType(listElementInstanceType,
					                                                           () => index.CompileTimeIndex(0, elements.Count, context)
					                                                                      .Branch(compileConstantIndex => compileConstantIndex < elements.Count
						                                                                                                      ? new Result<IValue>(elements[compileConstantIndex])
						                                                                                                      : context.Trace(EleMessageCode.ArgumentOutOfRange, $"Index {compileConstantIndex} out of range - list has {elements.Count} items"),
					                                                                              () =>
					                                                                              {
						                                                                              var operands = new Instruction[elements.Count];

						                                                                              for (var i = 0; i < elements.Count; i++)
						                                                                              {
							                                                                              // If any elements are not instructions then we need to 
							                                                                              if (elements[i].InnerIs(out Instruction instruction)) operands[i] = instruction;
							                                                                              else return new Result<IValue>(new HomogenousListElement(index, elements));
						                                                                              }

						                                                                              return Switch.CreateAndOptimize(index, operands, context).Cast<IValue>();
					                                                                              }), context));

			private HomogenousListElement(Instruction index, IReadOnlyList<IValue> elements)
			{
                _index = index;
                _elements = elements;
            }

            private readonly Instruction _index;
            private readonly IReadOnlyList<IValue> _elements;
            public string TypeOf => $"HomogenousListElement({_index})[{string.Join(", ", _elements.Select(e => e.TypeOf))}]";
            public string SummaryString => $"HomogenousListElement({_index})[{string.Join(", ", _elements)}]";

            private Result<IValue> ApplyToAllElements(Func<IValue, Result<IValue>> func, Context context)
	            => _elements.Select(func)
	                        .BindEnumerable(elements => Create(_index, elements.ToArray(), context));

            public IReadOnlyList<ResolvedPort> InputPorts => _elements[0].InputPorts;
            public IValue ReturnConstraint => _elements[0].ReturnConstraint;
            public IReadOnlyList<Identifier> Members => _elements[0].Members;
            public Result<IValue> Index(Identifier id, Context context) => ApplyToAllElements(e => e.Index(id, context), context);
            public Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) => ApplyToAllElements(e => e.Call(arguments.ToArray(), context), context);
            public Result<bool> MatchesConstraint(IValue value, Context context) => context.Trace(EleMessageCode.NotCompileConstant, "ListElement index is unknown at compile time - it cannot be used as a constraint");
            public Result<IValue> DefaultValue(Context context) => ApplyToAllElements(e => e.DefaultValue(context), context);
            public void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context) => context.Trace(EleMessageCode.NotCompileConstant, "ListElement index is unknown at compile time - it cannot be serialized");
            public Result<IValue> Deserialize(Func<Instruction> nextValue, Context context) => context.Trace(EleMessageCode.NotCompileConstant, "ListElement index is unknown at compile time - it cannot be used to deserialize");
            public Result<IValue> InstanceType(Context context) => _elements[0].InstanceType(context);
            public bool IsIntrinsicOfType<TIntrinsicImplementation>() where TIntrinsicImplementation : IIntrinsicImplementation => false;
            public bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic) => false;
		}
	}
}