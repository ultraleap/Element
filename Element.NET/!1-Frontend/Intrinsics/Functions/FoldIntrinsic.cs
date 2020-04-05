using System.Linq;

namespace Element.AST
{
	public sealed class FoldIntrinsic : IntrinsicFunction
	{
		public FoldIntrinsic()
			: base("List.fold",
			       new[]
			       {
				       new Port("list", ListType.Instance),
				       new Port("initial", AnyConstraint.Instance),
				       new Port("accumulator", BinaryFunctionConstraint.Instance)
			       }, Port.ReturnPort(AnyConstraint.Instance))
		{ }
		
		public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
		{
			var list = ListType.EvaluateElements(arguments[0] as StructInstance, compilationContext);
			var workingValue = arguments[1];
			var aggregator = (IFunctionSignature)arguments[2];
			return list.Aggregate(workingValue, (current, e) => aggregator.ResolveCall(new[] {current, e}, null, false, compilationContext));
		}
	}
}