namespace Element
{
	/// <summary>
	/// A type that accepts any other type, e.g. value, expression, function. The default type in Element.
	/// </summary>
	public sealed class AnyType : INamedType
	{
		public static INamedType Instance { get; } = new AnyType();
		PortInfo[] IFunction.Inputs => null;
		PortInfo[] IFunction.Outputs => null;
		private AnyType() { }
		public override string ToString() => "<any>";
		public string Name => "any";
		IFunction IFunction.CallInternal(IFunction[] arguments, string output, CompilationContext context) => this;
		bool? IType.SatisfiedBy(IFunction value, CompilationContext info) => true;
	}
}