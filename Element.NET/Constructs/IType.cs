namespace Element
{
	/// <summary>
	/// A type is an abstract function that can be used to constraint inputs and outputs.
	/// It could be a user-defined type, or a special one such as NumberType or AnyType.
	/// </summary>
	public interface IType : IFunction
	{
		/// <summary>
		/// Checks if the given value satisfies this type. The compilation may succeed or fail regardless,
		/// but the compiler may skip processing some sections if a check such as this fails.
		/// </summary>
		/// <param name="value">The value to check.</param>
		/// <param name="info">Message logger. Reasons for failure to satisfy will be added here.</param>
		/// <returns>True if it was satisfied, False if it was not satisfied, Null if it wasn't satisfied but
		/// the error should not be reported (e.g. and ErrorValue was passed in)</returns>
		bool? SatisfiedBy(IFunction value, CompilationContext info);
	}

	/// <summary>
	/// A type with a name, i.e. user-defined types.
	/// </summary>
	public interface INamedType : IType, INamedItem { }
}