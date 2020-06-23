namespace Element.AST
{
	public class ListIntrinsic : IIntrinsicFunction
	{
		public ListIntrinsic()
			: base("list",
			       new[] {Port.VariadicPort},
			       Port.ReturnPort(ListType.Instance))
		{ }

		public override Result<IValue> Call(IValue[] arguments, CompilationContext context) =>
			

		public Result<IValue> Call(IValue[] arguments) => ListType.Instance.MakeList(arguments);

		public Port[] Inputs { get; }
		public Port Output { get; }
	}
}