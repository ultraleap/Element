namespace Element
{
	/// <summary>
	/// A value that results from a failure during compilation. It will be accepted everywhere and generate no further
	/// errors, returning itself from each operation (the error is non-recoverable).
	/// </summary>
	public sealed class CompilationError : IFunction
	{
		public static IFunction Instance { get; } = new CompilationError();
		PortInfo[] IFunction.Inputs => null;
		PortInfo[] IFunction.Outputs => null;
		private CompilationError() { }
		public override string ToString() => "<error>";
		IFunction IFunction.CallInternal(IFunction[] arguments, string output, CompilationContext context) => this;
	}

	public interface IValue
	{
		string Identifier { get; }
	}

	/// <summary>
	/// A value that results from a failure during compilation. It will be accepted everywhere and generate no further
	/// errors, returning itself from each operation (the error is non-recoverable).
	/// </summary>
	public sealed class CompilationErr : IValue
	{
		public static IValue Instance { get; } = new CompilationErr();
		private CompilationErr() { }
		public override string ToString() => "<error>";

		public string Identifier => "<error>";
	}
}