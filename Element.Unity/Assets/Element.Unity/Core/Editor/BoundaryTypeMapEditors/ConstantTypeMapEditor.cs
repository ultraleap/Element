namespace Element.Unity
{
	using System;
	using UnityEditor;
	using UnityEngine;

	[BoundaryTypeMapEditor(typeof(ConstantTypeMap), "Constant", Order = -1)]
	public class ConstantTypeMapEditor : BoundaryTypeMapEditor
	{
		public override void OnGUI(IType type, int size, SerializedProperty value, SerializedProperty objValue)
		{
			switch (size)
			{
				case 4:
					EditorGUILayout.PropertyField(value, GUIContent.none);
					break;
				case 3:
					var v3 = (Vector3)value.vector4Value;
					v3 = EditorGUILayout.Vector3Field(GUIContent.none, v3);
					value.vector4Value = v3;
					break;
				case 2:
					var v2 = (Vector2)value.vector4Value;
					v2 = EditorGUILayout.Vector2Field(GUIContent.none, v2);
					value.vector4Value = v2;
					break;
				case 1:
					var f = value.vector4Value.x;
					f = EditorGUILayout.FloatField(GUIContent.none, f);
					value.vector4Value = new Vector4(f, 0, 0, 0);
					break;
				default: throw new NotSupportedException();
			}
		}
	}
}