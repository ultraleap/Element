namespace Element
{
	using System;

	/// <summary>
	/// The type for a single number.
	/// </summary>
	public sealed class NumberType : INamedType
	{
		// TODO: IEvaluable here?
		public static INamedType Instance { get; } = new NumberType();
		PortInfo[] IFunction.Inputs => Array.Empty<PortInfo>();
		PortInfo[] IFunction.Outputs => Array.Empty<PortInfo>();
		private NumberType() { }
		public override string ToString() => "<number>";
		public string Name => "num";

		IFunction IFunction.CallInternal(IFunction[] arguments, string output, CompilationContext context) =>
			context.LogError(9999, "Tried to call a leaf value");

		bool? IType.SatisfiedBy(IFunction value, CompilationContext info) =>
			value is CompileError ? (bool?)null : (value.Inputs?.Length == 0 && value.Outputs?.Length == 0);
	}
}