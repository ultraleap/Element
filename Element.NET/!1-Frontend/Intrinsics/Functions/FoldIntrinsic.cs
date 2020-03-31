using System.Linq;

namespace Element.AST
{
	public class FoldIntrinsic : IIntrinsic, IFunction
	{
		public IType Type => FunctionType.Instance;
		public string Location => "List.fold";
		public Port[] Inputs { get; } =
		{
			new Port("list", ListType.Instance),
			new Port("initial", AnyConstraint.Instance),
			new Port("accumulator", FunctionType.Instance)
		};
		public Port Output { get; } = Port.ReturnPort(AnyConstraint.Instance);
		public IValue Call(IValue[] arguments, CompilationContext compilationContext)
		{
			var list = ListType.EvaluateElements(arguments[0] as IScope, compilationContext);
			var workingValue = arguments[1];
			var aggregator = (ICallable)arguments[2];
			return list.Aggregate(workingValue, (current, e) => aggregator.Call(new[] {current, e}, compilationContext));
		}
	}
}