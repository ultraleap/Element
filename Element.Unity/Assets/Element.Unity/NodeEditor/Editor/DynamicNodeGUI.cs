namespace Element.Unity.NodeEditor
{
	using XNodeEditor;
	using System.Linq;
	using System;

	[CustomNodeEditor(typeof(ElementNodeBase))]
	public class DynamicNodeGUI : NodeEditor
	{
		public override void OnBodyGUI()
		{
			base.OnBodyGUI();
			var inputs = target.InstanceInputs.ToArray();
			var outputs = target.InstanceOutputs.ToArray();
			for (var i = 0; i < Math.Min(inputs.Length, outputs.Length); i++)
			{
				NodeEditorGUILayout.PortPair(inputs[i], outputs[i]);
			}

			for (var i = outputs.Length; i < inputs.Length; i++)
			{
				NodeEditorGUILayout.PortField(inputs[i]);
			}

			for (var i = inputs.Length; i < outputs.Length; i++)
			{
				NodeEditorGUILayout.PortField(outputs[i]);
			}
		}
	}
}