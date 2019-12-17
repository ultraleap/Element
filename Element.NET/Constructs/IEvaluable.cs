namespace Element
{
	/// <summary>
	/// A construct which can be evaluated as an expression
	/// </summary>
	public interface IEvaluable
	{
		Expression AsExpression(CompilationContext info);
	}
}