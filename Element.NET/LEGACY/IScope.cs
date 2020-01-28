namespace Element
{
	/// <summary>
	/// A scope represents a level of function/type abstractness within an element program.
	/// A scope can be used to compile functions with a given compilation stack and retrieve types.
	/// A scope could be a single file, group of files, or a function with a body.
	/// Scopes have no inputs (beyond those already specified at a level above).
	/// </summary>
	public interface IScope
	{
		/// <summary>
		/// Retrieves a Function value from the scope.
		/// </summary>
		/// <param name="name">The function name to compile</param>
		/// <param name="compilerStackFrame">Stack to compile within</param>
		/// <param name="context">Where to log messages</param>
		/// <returns>The compiled function or an error</returns>
		IFunction CompileFunction(string name, CompilerStackFrame compilerStackFrame, CompilationContext context);

		/// <summary>
		/// Retrieves a named Type from the scope or one of its parents.
		/// </summary>
		/// <param name="name">The type name</param>
		/// <param name="context">Where to log messages</param>
		/// <returns>The found type, or null</returns>
		INamedType FindType(string name, CompilationContext context);
	}
}