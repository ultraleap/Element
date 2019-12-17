namespace Element.Unity
{
	using System;
	using UnityEditor;

	public class Vector4View : IDebugView
	{
		private static float ToNumber(IFunction value)
		{
			return ((Constant)ConstantFolding.Optimize((Expression)value)).Value;
		}

		public void OnGUI(string name, IFunction value, CompilerInfo info, Action<string, IFunction> drawOther)
		{
			EditorGUILayout.LabelField(name,
				$"x: {ToNumber(value.Call("x", info))}, y: {ToNumber(value.Call("y", info))}, z: {ToNumber(value.Call("z", info))}, w: {ToNumber(value.Call("w", info))}");
		}

		public bool Supports(IFunction value)
		{
			return ElementContext.Instance.GetContext()
			                     .GetType("Vec4", null)
			                     .SatisfiedBy(value, new CompilerInfo{Logger = null}) == true;
		}
	}
}