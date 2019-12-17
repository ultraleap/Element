namespace Element.Unity.NodeEditor
{
	using XNode;
	using System;
	using System.Linq;
	using System.Collections.Generic;

	[CreateNodeMenu("Element/Decompose")]
	public class ElementDecomposeNode : ElementNodeBase
	{
		[Input] public ElementNodeValue Object;

		protected override void Init()
		{
			UpdatePorts();
		}

		protected void OnValidate()
		{
			name = Object.Value?.ToString() ?? "<Decompose>";
			UpdatePorts();
		}

		public void UpdatePorts()
		{
			var function = ((ElementNodeValue)GetInputPort(nameof(Object)).GetInputValue())?.Value;
			if (function == null || function.Inputs.Length > 0) { return; }
			var existingNames = new HashSet<string>(Outputs.Select(p => p.fieldName));
			foreach (var output in function.Outputs)
			{
				// TODO: Different types for different Element types
				var type = typeof(ElementNodeValue);
				if (existingNames.Remove(output.Name)) {
					GetPort(output.Name).ValueType = type;
				} else {
					AddInstanceOutput(type, ConnectionType.Multiple, output.Name);
				}
			}
			foreach (var name in existingNames) {
				RemoveInstancePort(name);
			}
		}

		public override object GetValue(NodePort port)
		{
			var function = Object.Value;
			if (function == null) { return null; }
			var info = new CompilerInfo{};
			return new ElementNodeValue{
				Value = function.Call(port.fieldName, info)
			};
		}

		public override string Evaluate(NodePort port, Func<NodePort, string> others)
		{
			return $"{others(Inputs.First())}.{port.fieldName}";
		}

		public override void OnCreateConnection(NodePort from, NodePort to)
		{
			UpdatePorts();
		}

		public override void OnRemoveConnection(NodePort port)
		{
			UpdatePorts();
		}
	}
}