namespace Element
{
	using System.Linq;

	public class PersistIntrinsic : INamedFunction
	{
		public string Name => "persist";

		public PortInfo[] Inputs { get; } =
		{
			new PortInfo {Name = "initial_value", Type = SerializableType.Instance},
			new PortInfo {Name = "new_value_function", Type = FunctionType.Instance}
		};

		public PortInfo[] Outputs { get; } =
		{
			new PortInfo {Name = "return", Type = AnyType.Instance}
		};

		public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
		{
			if (this.CheckArguments(arguments, output, context) != null)
			{
				return Error.Instance;
			}

			var initial = arguments[0];
			var newValue = arguments[1];
			var group = new Persist(initial.Serialize(context),
				state => newValue.Call(new[] {initial.Deserialize(state, context)}, context).Serialize(context));
			return initial.Deserialize(Enumerable.Range(0, group.Size).Select(i => new ExpressionGroupElement(group, i)), context);
		}
	}
}