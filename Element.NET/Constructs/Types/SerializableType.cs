namespace Element
{
	/// <summary>
	/// A type that accepts only serializable data.
	/// </summary>
	public sealed class SerializableType : IType
	{
		public static IType Instance { get; } = new SerializableType();
		PortInfo[] IFunction.Inputs => null;
		PortInfo[] IFunction.Outputs => null;
		private SerializableType() { }
		public override string ToString() => "<serializable type>";
		IFunction IFunction.CallInternal(IFunction[] arguments, string output, CompilationContext context) => this;

		bool? IType.SatisfiedBy(IFunction value, CompilationContext info) =>
			value is Error ? (bool?)null : value.GetSize(info).HasValue;
	}
}