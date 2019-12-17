namespace Element.Unity
{
	using System;
	using UnityEditor;

	public class TreeView : IDebugView
	{
		public void OnGUI(string name, IFunction value, CompilerInfo info, Action<string, IFunction> drawOther)
		{
			EditorGUILayout.LabelField(name);
			EditorGUI.indentLevel++;
			foreach (var o in value.Outputs)
			{
				drawOther(o.Name, value.Call(o.Name, info));
			}

			EditorGUI.indentLevel--;
		}

		public bool Supports(IFunction value) => value.Serialize(new CompilerInfo{Logger = null}).Length > 1;
	}
}