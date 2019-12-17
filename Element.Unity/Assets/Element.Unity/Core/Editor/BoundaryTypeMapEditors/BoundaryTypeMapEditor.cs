namespace Element.Unity
{
	using System;
	using UnityEditor;

	[AttributeUsage(AttributeTargets.Class, AllowMultiple = true)]
	public class BoundaryTypeMapEditorAttribute : Attribute
	{
		public Type Type { get; set; }
		public string Name { get; set; }
		public int Order { get; set; }

		public BoundaryTypeMapEditorAttribute(Type type, string name)
		{
			Type = type;
			Name = name;
		}
	}

	public abstract class BoundaryTypeMapEditor
	{
		public abstract void OnGUI(IType type, int size, SerializedProperty value, SerializedProperty objValue);
	}

	[BoundaryTypeMapEditor(typeof(TimeValue), "Time")]
	public class TimeValueEditor : BoundaryTypeMapEditor
	{
		public override void OnGUI(IType type, int size, SerializedProperty value, SerializedProperty objValue)
		{
		}
	}
}