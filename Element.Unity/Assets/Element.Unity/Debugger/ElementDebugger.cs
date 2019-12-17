namespace Element.Unity.Debugger
{
	using UnityEngine;

	[RequireComponent(typeof(IElementDebuggable))]
	public class ElementDebugger : MonoBehaviour
	{
		public IFunction DebugFunction => GetComponent<IElementDebuggable>().DebugFunction;
	}
}