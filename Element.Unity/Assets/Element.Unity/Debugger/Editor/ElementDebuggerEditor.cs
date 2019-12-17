namespace Element.Unity.Debugger
{
	using System;
	using System.Collections.Generic;
	using System.Globalization;
	using UnityEditor;

	[CustomEditor(typeof(ElementDebugger))]
	public class ElementDebuggerEditor : Editor
	{
		private readonly HashSet<string> _openedFoldouts = new HashSet<string>();
		private int _nestedness;
		private readonly Dictionary<string, (IFunction func, Func<float> funcCompiled)> _funcCache = new Dictionary<string, (IFunction, Func<float>)>();

		private IFunction _previous;
		private IFunction _calledFunc;

		public override void OnInspectorGUI()
		{
			var info = new CompilerInfo {DebugMode = true, Logger = UnityLoggers.HelpBox};
			var func = (target as ElementDebugger)?.DebugFunction;
			if (func != _previous)
			{
				// TODO: Generalise and/or parameterise this so Time and Index are used
				// Move to implementations of IElementDebuggable
				_calledFunc = func?.Call(new[] {Constant.Zero}, info);
				_previous = func;
			}
			_nestedness = 0;
			_calledFunc?.RecursiveAction(Array.Empty<IFunction>(), "", ref _nestedness, info, DrawFoldout, RecursePredicate);
		}

		private void DrawFoldout(string functionName, IFunction func)
		{
			EditorGUI.indentLevel = _nestedness;

			var foldoutId = FoldoutID(functionName, func);
			var displayName = $"{SplitByLastIndexOf(functionName, '.')} = {(func as IDebuggable)?.DebugName}";

			var info = new CompilerInfo {DebugMode = true, Logger = UnityLoggers.HelpBox};
			var expr = func.AsExpression(info);
			float? value = null;
			if (expr != null)
			{
				if (!_funcCache.TryGetValue(functionName, out var funcPair) || funcPair.func != func)
				{
					funcPair = (func, func.Compile<Func<float>>());
					_funcCache[functionName] = funcPair;
				}

				value = funcPair.funcCompiled?.Invoke();
			}


			var foldoutLabel = string.IsNullOrEmpty(functionName)
				? _nestedness == 0
					? (func as IDebuggable)?.DebugName
					: "Unnamed Function"
				: $"{displayName} <{(value == null ? "Unable to show" : value.Value.ToString(CultureInfo.InvariantCulture))}>";
			_openedFoldouts.SetPresent(foldoutId, EditorGUILayout.Foldout(RecursePredicate((functionName, func)), foldoutLabel));
		}

		private bool RecursePredicate((string functionName, IFunction func) f) => _openedFoldouts.Contains(FoldoutID(f.functionName, f.func));

		private static string SplitByLastIndexOf(string str, char character)
		{
			var idx = str.LastIndexOf(character);
			return idx != -1 ? str.Substring(idx + 1) : str;
		}

		private static string FoldoutID(string name, IFunction func) => $"{(func as IDebuggable)?.DebugName}:{name}";
	}
}