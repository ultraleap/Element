namespace Element.Unity
{
	using System;
	using System.Collections.Generic;
	using System.Linq;
	using UnityEditor;
	using UnityEngine;

	[CustomPropertyDrawer(typeof(FunctionNameAttribute))]
	public class FunctionNameDrawer : PropertyDrawer
	{
		private static readonly Dictionary<(string, FunctionClass), string[]> _names
			= new Dictionary<(string, FunctionClass), string[]>();

		static FunctionNameDrawer()
		{
			ElementContext.NewContext += () => _names.Clear();
		}

		public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
		{
			var attr = ((FunctionNameAttribute)attribute);
			DrawGUI(attr, position, property, label);
		}

		public static void DrawGUI(FunctionNameAttribute attr, Rect position, SerializedProperty property, GUIContent label)
		{
			var key = (attr.Type, attr.Class);

			if (!_names.TryGetValue(key, out var names))
			{
				var contextObj = (ElementContext)property.serializedObject.FindProperty("Context")?.objectReferenceValue
					?? ElementContext.Instance;
				if (contextObj == null) {
					EditorGUI.HelpBox(position, $"No Element Context", MessageType.Error);
					return;
				}
				var context = contextObj.GetContext();
				var info1 = new CompilerInfo{Logger = null};
				var requiredType = context.GetType(attr.Type, info1);
				if (requiredType == null)
				{
					EditorGUI.HelpBox(position, $"Type {attr.Type} not found", MessageType.Error);
					return;
				}
				IEnumerable<(string, IFunction)> fList;
				switch (attr.Class)
				{
					case FunctionClass.FunctionWithConfig:
						fList = context.GetAllFunctions(f =>
						{
							if (f.Inputs == null || f.Outputs?.Length != 1)
							{
								return false;
							}
							return f.Outputs[0].Type == requiredType;
						});
						break;

					case FunctionClass.Function:
						fList = context.GetAllFunctions(f => 
							requiredType.SatisfiedBy(f, info1) == true);
						break;

					case FunctionClass.Constructor:
						fList = context.Types.ToArray()
							.Where(t => t.Inputs?.Length == 0)
							.Select(i => context.GetFunction(i.Name))
							.Where(f => f != null)
							.Select(f => (f.Name, (IFunction)f));
						break;

					case FunctionClass.Type:
						fList = context.Types.ToArray()
							.Select(f => (f.Name, (IFunction)f));
						break;
					default: throw new ArgumentOutOfRangeException();
				}
				names = fList.Select(b => b.Item1.Replace('.', '/')).ToArray();
				_names.Add(key, names);
			}

			var index = Array.IndexOf(names, property.stringValue.Replace('.', '/'));
			position = EditorGUI.PrefixLabel(position, label);
			index = EditorGUI.Popup(position, index, names);
			property.stringValue = index < 0 ? "" : names[index].Replace('/', '.');
		}
	}
}