namespace Element.Unity
{
	using System.Globalization;
	using System.IO;
	using System.Linq;
	using UnityEngine;
	using UnityEditor;
	using System.Reflection;

	public class ErrorsDrawer
	{
		private static class Styles
		{
			static Styles()
			{
				var LoadIcon = typeof(EditorGUIUtility)
					.GetMethod("LoadIcon", BindingFlags.NonPublic | BindingFlags.Static);
				errorIcon = (Texture2D)LoadIcon.Invoke(null, new []{"console.erroricon.sml"});
				warningIcon = (Texture2D)LoadIcon.Invoke(null, new []{"console.warnicon.sml"});
			}

			public static Texture2D errorIcon;
			public static Texture2D warningIcon;
			public static GUIStyle messageStyle = "CN StatusInfo";
			public static GUIStyle evenBackground = "CN EntryBackEven";
		}

		private struct ElementError
		{
			public string message;
			public string messageDetails;
			public string platform;
			public string file;
			public int line;
			public int warning;
		}

		private static void ElementErrorListUI(ElementError[] errors, ref Vector2 scrollPosition)
		{
			GUILayout.Space(5f);
			int num = errors.Length;
			GUILayout.Label($"Errors ({num}):", EditorStyles.boldLabel);
			int controlID = GUIUtility.GetControlID("ElementErrorView".GetHashCode(), FocusType.Passive);
			float minHeight = Mathf.Min((float)num * 20f + 40f, 150f);
			scrollPosition = GUILayout.BeginScrollView(scrollPosition, /*GUISkin.current.box,*/ GUILayout.MinHeight(minHeight));
			EditorGUIUtility.SetIconSize(new Vector2(16f, 16f));
			float height = Styles.messageStyle.CalcHeight(new GUIContent(Styles.errorIcon), 100f);
			Event current = Event.current;
			for (int i = 0; i < num; i++)
			{
				Rect controlRect = EditorGUILayout.GetControlRect(false, height);
				string message = errors[i].message;
				string platform = errors[i].platform;
				bool flag = errors[i].warning != 0;
				string lastPathNameComponent = Path.GetFileName(errors[i].file);
				int line = errors[i].line;
				if (current.type == EventType.MouseDown && current.button == 0 && controlRect.Contains(current.mousePosition))
				{
					GUIUtility.keyboardControl = controlID;
					if (current.clickCount == 2)
					{
						string file = errors[i].file;

						// TODO: A better way of locating the .ele files:
						file = AssetDatabase.FindAssets(file)
							.Select(AssetDatabase.GUIDToAssetPath)
							.Where(p => Path.GetFileNameWithoutExtension(p) == file
								&& p.EndsWith(".ele", true, CultureInfo.InvariantCulture))
							.FirstOrDefault();

						Object asset = (!string.IsNullOrEmpty(file)) ? AssetDatabase.LoadMainAssetAtPath(file) : null;
						if (asset != (Object)null)
						{
							AssetDatabase.OpenAsset(asset, line);
						}
						GUIUtility.ExitGUI();
					}
					current.Use();
				}
				if (current.type == EventType.ContextClick && controlRect.Contains(current.mousePosition))
				{
					current.Use();
					GenericMenu genericMenu = new GenericMenu();
					int errorIndex = i;
					genericMenu.AddItem(EditorGUIUtility.TrTextContent("Copy error text", (string)null, (Texture)null), false, delegate
					{
						// string text = errors[errorIndex].message;
						// if (!string.IsNullOrEmpty(errors[errorIndex].messageDetails))
						// {
						// 	text += '\n';
						// 	text += errors[errorIndex].messageDetails;
						// }
						EditorGUIUtility.systemCopyBuffer = errors[errorIndex].messageDetails;
					});
					genericMenu.ShowAsContext();
				}
				if (current.type == EventType.Repaint && (i & 1) == 0)
				{
					Styles.evenBackground.Draw(controlRect, false, false, false, false);
				}
				Rect rect = controlRect;
				rect.xMin = rect.xMax;
				if (line > 0)
				{
					GUIContent content = (!string.IsNullOrEmpty(lastPathNameComponent))
						? new GUIContent(lastPathNameComponent + ":" + line.ToString())
						: new GUIContent(line.ToString());
					Vector2 vector = EditorStyles.miniLabel.CalcSize(content);
					rect.xMin -= vector.x;
					GUI.Label(rect, content, EditorStyles.miniLabel);
					rect.xMin -= 2f;
					if (rect.width < 30f)
					{
						rect.xMin = rect.xMax - 30f;
					}
				}
				Rect position = rect;
				position.width = 0f;
				if (platform.Length > 0)
				{
					GUIContent content2 = new GUIContent(platform);
					Vector2 vector2 = EditorStyles.miniLabel.CalcSize(content2);
					position.xMin -= vector2.x;
					Color contentColor = GUI.contentColor;
					GUI.contentColor = new Color(1f, 1f, 1f, 0.5f);
					GUI.Label(position, content2, EditorStyles.miniLabel);
					GUI.contentColor = contentColor;
					position.xMin -= 2f;
				}
				Rect position2 = controlRect;
				position2.xMax = position.xMin;
				GUI.Label(position2, new GUIContent(message, (!flag) ? Styles.errorIcon : Styles.warningIcon), Styles.messageStyle);
			}
			EditorGUIUtility.SetIconSize(Vector2.zero);
			GUILayout.EndScrollView();
		}

		private Vector2 scrollPosition;

		public void OnGUI(CompilationResult obj)
		{
			var errorList = obj.Errors.Select(e => {
				var lines = e.Split('\n');
				var location = (lines.Length > 2 ? lines[2].Trim() : "null:0,0").Split(':');
				var lineColumn = location.Length > 1 ? location[1].Split(',') : new[]{"0","0"};
				return new ElementError
				{
					message = lines[0],
					messageDetails = e,
					file = location[0],
					line = int.Parse(lineColumn[0]),
					platform = ""
				};
			}).ToArray();
			if (errorList.Length > 0)
				ElementErrorListUI(errorList, ref scrollPosition);
		}
	}
}