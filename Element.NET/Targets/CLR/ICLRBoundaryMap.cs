namespace Element.CLR
{
	using System;

	public delegate System.Linq.Expressions.Expression? CompileFunction(IFunction function, Type outputType, CompilationContext context);

	/// <summary>
	/// Provides mapping for CLR types to and from Element types.
	/// </summary>
	public interface ICLRBoundaryMap
	{
		/// <summary>
		/// Converts a target Expression to an IFunction using the given type map.
		/// </summary>
		IFunction ToInput(System.Linq.Expressions.Expression parameter, ICLRBoundaryMap rootTypeMap, CompilationContext context);

		/// <summary>
		/// Converts an IFunctions to a target Expression using the given compilation function and compiler context.
		/// </summary>
		System.Linq.Expressions.Expression? FromOutput(IFunction output, Type outputType,
		                                               CompileFunction compile,
		                                               CompilationContext context);
	}
}