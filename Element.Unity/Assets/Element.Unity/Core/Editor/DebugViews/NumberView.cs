namespace Element.Unity
{
	using System;
	using UnityEditor;

	public class NumberView : IDebugView
	{
		public void OnGUI(string name, IFunction value, CompilerInfo info, Action<string, IFunction> drawOther)
		{
			EditorGUILayout.LabelField(name, ((Constant)ConstantFolding.Optimize((Expression)value)).Value.ToString());
		}

		public bool Supports(IFunction value) => value is Expression;
	}
}