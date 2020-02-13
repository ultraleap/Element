using Element.AST;

namespace Element
{
	/// <summary>
	/// An intrinsic with 2 arguments and a single return.
	/// </summary>
	internal class BinaryIntrinsic : INamedFunction, ICallable
	{
		public BinaryIntrinsic(Binary.Op operation)
		{
			Name = operation.ToString().ToLowerInvariant();
			Identifier = new Identifier(Name);
			Operation = operation;
		}

		public string Name { get; } // TODO: Remove name in favor of Identifier
		public Identifier Identifier { get; }
		public Binary.Op Operation { get; }

		public PortInfo[] Inputs { get; } =
		{
			new PortInfo {Name = "a", Type = NumberType.Instance},
			new PortInfo {Name = "b", Type = NumberType.Instance}
		};

		Port[] ICallable.Inputs { get; } =
			// TODO: Don't implement explicitly
		{
			// TODO: Add type.
			new Port {Identifier = new Identifier("a")},
			new Port {Identifier = new Identifier("b")},
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

		public bool CanBeCached => true;
		public IValue Call(IValue[] arguments, CompilationFrame frame, CompilationContext compilationContext) =>
			compilationContext.CheckArguments(arguments, (this as ICallable).Inputs)
				? new Literal(Binary.Evaluate(Operation, arguments[0] as Literal, arguments[1] as Literal))
				: (IValue) CompilationErr.Instance;
	}
}