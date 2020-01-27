namespace Element
{
	using System;
	using System.Linq;

	/// <summary>
	/// A Function that creates a structure based on the outputs of a type.
	/// For example, `Vec3(1, 2, 3)` is calling the constructor; the vector's
	/// x, y, and z outputs (in that order) are mapped to the given inputs.
	/// </summary>
	internal class Constructor : INamedFunction
	{
		/// <summary>
		/// Creates a constructor Function based on the given type
		/// </summary>
		public Constructor(IFunction type, CompilationContext info)
		{
			if (type.Inputs?.Length != 0)
			{
				throw new Exception($"Can't make a constructor for {type} because it has inputs");
			}

			if (type.Outputs == null)
			{
				throw new Exception($"Can't make a constructor for {type} because it's not introspectable");
			}

			Type = type;
			Name = ((INamedItem)type).Name;
			Inputs = Type.Outputs.Select(o => new PortInfo {Name = o.Name, Type = (IType)Type.Call(o.Name, info)})
			             .ToArray();
			Outputs = Type.Outputs;
		}

		public IFunction Type { get; }
		public string Name { get; }
		public PortInfo[] Inputs { get; }
		public PortInfo[] Outputs { get; }

		public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context) =>
			this.CheckArguments(arguments, output, context) ?? arguments[Array.FindIndex(Outputs, p => p.Name == output)];

		public override string ToString() => Name;
	}
}