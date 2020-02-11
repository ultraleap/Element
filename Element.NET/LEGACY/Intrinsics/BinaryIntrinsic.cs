using System;
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
			Operation = operation;
		}

		public string Name { get; }
		public Binary.Op Operation { get; }

		public PortInfo[] Inputs { get; } =
		{
			new PortInfo {Name = "a", Type = NumberType.Instance},
			new PortInfo {Name = "b", Type = NumberType.Instance}
		};

		Port[] ICallable.Inputs { get; } =
		{
			// No type checking yet, only length checking so who cares what they are? :haHAA:
			// TODO: Don't do this.
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
		public IValue Call(Func<IValue>[] arguments, CompilationFrame frame, CompilationContext compilationContext)
		{
			if (!compilationContext.CheckArguments(arguments, (this as ICallable).Inputs)) return CompilationErr.Instance;
			var a = arguments[0]?.Invoke();
			var b = arguments[1]?.Invoke();
			return new Literal(Binary.Evaluate(Operation, a as Literal, b as Literal));
		}
	}
}