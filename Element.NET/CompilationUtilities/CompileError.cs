namespace Element
{
	/// <summary>
	/// A value that results from a failure during compilation. It will be accepted everywhere and generate no further
	/// errors, returning itself from each operation (the error is non-recoverable).
	/// </summary>
	public sealed class CompileError : IFunction
	{
		public static IFunction Instance { get; } = new CompileError();
		PortInfo[] IFunction.Inputs => null;
		PortInfo[] IFunction.Outputs => null;
		private CompileError() { }
		public override string ToString() => "<error>";
		IFunction IFunction.CallInternal(IFunction[] arguments, string output, CompilationContext context) => this;
	}
}