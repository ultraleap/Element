using Element.AST;

namespace Element
{
	/// <summary>
	/// An abstract Function, that is a thing that Element deals with in its compilation pipeline.
	/// It will typically have Inputs and Outputs and can be Called, but may be none of those things (e.g. Expression)
	/// </summary>
	public interface IFunction
	{
		/// <summary>
		/// An ordered list of input ports.
		/// </summary>
		/// <value>Can be null, which means 'unknown'. Do not modify the return value.</value>
		PortInfo[] Inputs { get; }

		/// <summary>
		/// An ordered list of output ports. To introspect their types you should use Call.
		/// </summary>
		/// <value>Can be null, which means 'unknown'. Do not modify the return value.</value>
		PortInfo[] Outputs { get; }

		/// <summary>
		/// Call the function, that is, provide inputs and retrieve an output.
		/// </summary>
		/// <param name="arguments">The ordered list of inputs. NB these can be values or Types.</param>
		/// <param name="output">The output port name to evaluate.</param>
		/// <param name="info">Message logger.</param>
		/// <returns>A Function value, a Type, or ErrorValue depending on the result of the call.</returns>
		IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context);
	}

	/// <summary>
	/// A Function with a name, i.e. most intrinsics and user-defined functions.
	/// </summary>
	public interface INamedFunction : IFunction, INamedItem { }
}