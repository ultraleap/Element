namespace Element
{
	internal class UnaryIntrinsic : INamedFunction
	{
		public UnaryIntrinsic(Unary.Op operation)
		{
			Name = operation.ToString().ToLowerInvariant();
			Operation = operation;
		}

		public string Name { get; }
		public Unary.Op Operation { get; }

		public PortInfo[] Inputs { get; } =
		{
			new PortInfo {Name = "a", Type = NumberType.Instance}
		};

		public PortInfo[] Outputs { get; } = {
			new PortInfo{Name = "return", Type = NumberType.Instance}
		};

		public IFunction CallInternal(IFunction[] arguments, string name, CompilationContext context)
		{
			var a = arguments[0].AsExpression(context);
			return this.CheckArguments(arguments, name, context)
				?? (a == null ? (IFunction)NumberType.Instance : new Unary(Operation, a));
		}
	}
}