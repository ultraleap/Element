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
			ListType.Instance.MakeList(arguments, context);
	}
}