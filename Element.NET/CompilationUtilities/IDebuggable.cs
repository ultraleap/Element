namespace Element
{
	public interface IDebuggable
	{
		string[] IntermediateValues { get; }
		IFunction CompileIntermediate(IFunction[] arguments, string name, CompilationContext context);
		string DebugName { get; }
	}
}