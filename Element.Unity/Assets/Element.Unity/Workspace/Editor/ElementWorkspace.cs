namespace Element.Unity.Workspace
{
	using UnityEditor;
	using UnityEngine;
	using System;
	using System.Diagnostics;
	using System.Linq;
	using static UnityEditor.EditorGUILayout;

	public class ElementWorkspace : EditorWindow
	{
		[MenuItem("Window/Element workspace")]
		public static void Open()
		{
			GetWindow<ElementWorkspace>("Element workspace", true,
				typeof(SceneView).Assembly.GetType("UnityEditor.InspectorWindow"));
		}

		public string Code = "";

		[NonSerialized] private string _previous;
		private IFunction value;

		private static IDebugView[] Viewers =
		{
			new NumberView(),
			new Vector4View(),
			new Vector3View(),
			new PathView(),
			new TreeView()
		};

		private readonly Stopwatch _evaluationTimer = new Stopwatch();
		private const float _evaluationTick = 0.1f;

		private static readonly CompilerInfo errorGui = new CompilerInfo{Logger = UnityLoggers.HelpBox};

		private void OnEnable()
		{
			_evaluationTimer.Start();
		}

		protected void OnGUI()
		{
			Code = TextArea(Code, GUILayout.Height(200));
			try
			{
				if (_evaluationTimer.Elapsed.TotalSeconds > _evaluationTick && Code != _previous)
				{
					value = ElementContext.Instance.GetContext().Parse("_{\n" + Code + "\n}");
					_evaluationTimer.Restart();
					_previous = Code;
				}
			}
			catch (Exception e)
			{
				HelpBox(e.ToString(), MessageType.Error);
			}

			
			if (value != null)
			{
				var context = (IContext)value;
				var stack = new FunctionStack();
				foreach (var name in context.DriverNames)
				{
					var driver = context.CompileDriver(name, stack, errorGui);
					DrawFunction(name, driver);
				}
			}
		}

		private void DrawFunction(string name, IFunction driver)
		{
			var viewer = Viewers.FirstOrDefault(v => v.Supports(driver));
			if (viewer == null)
			{
				HelpBox($"Unable to visualise {name}", MessageType.Warning);
			} else {
				viewer.OnGUI(name, driver, errorGui, DrawFunction);
			}
		}
    }
}