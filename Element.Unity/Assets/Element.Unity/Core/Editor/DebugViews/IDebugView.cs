namespace Element.Unity
{
	using System;

	public interface IDebugView
	{
		bool Supports(IFunction value);
		void OnGUI(string name, IFunction value, CompilerInfo info, Action<string, IFunction> drawOther);
	}
}