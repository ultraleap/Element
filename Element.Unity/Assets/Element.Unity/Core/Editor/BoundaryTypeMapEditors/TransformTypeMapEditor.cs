namespace Element.Unity
{
	using UnityEditor;
	using UnityEngine;

	[BoundaryTypeMapEditor(typeof(PositionTypeMap), "Position")]
	[BoundaryTypeMapEditor(typeof(DirectionTypeMap), "Direction")]
	[BoundaryTypeMapEditor(typeof(MatrixTypeMap), "Matrix")]
	public class TransformTypeMapEditor : BoundaryTypeMapEditor
	{
		public override void OnGUI(IType type, int size, SerializedProperty value, SerializedProperty objValue)
		{
			objValue.objectReferenceValue = EditorGUILayout.ObjectField(
				objValue.objectReferenceValue, typeof(Transform), true);
		}
	}
}