namespace Element
{
	/// <summary>
	/// A type that accepts only functions with inputs.
	/// </summary>
	public sealed class FunctionType : IType
	{
		public static IType Instance { get; } = new FunctionType();
		PortInfo[] IFunction.Inputs => null;
		PortInfo[] IFunction.Outputs => null;
		private FunctionType() { }
		public override string ToString() => "<function with inputs>";
		IFunction IFunction.CallInternal(IFunction[] arguments, string output, CompilationContext context) => AnyType.Instance;

		bool? IType.SatisfiedBy(IFunction value, CompilationContext info)
			=> value is CompileError ? (bool?)null : (value == this || value.Inputs?.Length > 0);
	}
}