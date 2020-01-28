namespace Element
{
	using System.Linq;

	public class ForIntrinsic : INamedFunction
	{
		public string Name => "for";

		public PortInfo[] Inputs { get; } =
		{
			new PortInfo {Name = "initial", Type = SerializableType.Instance},
			new PortInfo {Name = "condition", Type = FunctionType.Instance},
			new PortInfo {Name = "body", Type = FunctionType.Instance}
		};

		public PortInfo[] Outputs { get; } =
		{
			new PortInfo {Name = "return", Type = AnyType.Instance}
		};

		public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
		{
			if (this.CheckArguments(arguments, output, context) != null)
			{
				return CompilationError.Instance;
			}

			var initial = arguments[0];
			if (arguments.Any(a => a is IType))
			{
				return initial;
			}

			var initialSerialized = initial.Serialize(context);
			var condition = arguments[1];
			var body = arguments[2];
			var group = new Loop(initialSerialized,
				state => condition.Call(new[] {initial.Deserialize(state, context)}, context).AsExpression(context),
				state => body.Call(new[] {initial.Deserialize(state, context)}, context).Serialize(context));
			return initial.Deserialize(Enumerable.Range(0, group.Size).Select(i => new ExpressionGroupElement(group, i)), context);
		}
	}
}