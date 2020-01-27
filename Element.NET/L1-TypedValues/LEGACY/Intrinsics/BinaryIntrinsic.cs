namespace Element
{
	/// <summary>
	/// An intrinsic with 2 arguments and a single return.
	/// </summary>
	internal class BinaryIntrinsic : INamedFunction
	{
		public BinaryIntrinsic(Binary.Op operation)
		{
			Name = operation.ToString().ToLowerInvariant();
			Operation = operation;
		}

		public string Name { get; }
		public Binary.Op Operation { get; }

		public PortInfo[] Inputs { get; } =
		{
			new PortInfo {Name = "a", Type = NumberType.Instance},
			new PortInfo {Name = "b", Type = NumberType.Instance}
		};

		public PortInfo[] Outputs { get; } =
		{
			new PortInfo {Name = "return", Type = NumberType.Instance}
		};

		public IFunction CallInternal(IFunction[] arguments, string name, CompilationContext context)
		{
			var a = arguments[0].AsExpression(context);
			var b = arguments[1].AsExpression(context);
			return this.CheckArguments(arguments, name, context)
				?? ((a == null || b == null) ? (IFunction)NumberType.Instance : new Binary(Operation, a, b));
		}
	}
}