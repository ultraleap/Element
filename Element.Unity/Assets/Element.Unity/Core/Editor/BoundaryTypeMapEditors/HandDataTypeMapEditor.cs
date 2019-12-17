namespace Element.Unity
{
	using UnityEditor;

	[BoundaryTypeMapEditor(typeof(HandDataTypeMap), "HandData")]
	public class HandDataTypeMapEditor : BoundaryTypeMapEditor
	{
		public override void OnGUI(IType type, int size, SerializedProperty value, SerializedProperty objValue)
		{
			objValue.objectReferenceValue = EditorGUILayout.ObjectField(
				objValue.objectReferenceValue, typeof(HandDataProvider), true);
		}
	}
}